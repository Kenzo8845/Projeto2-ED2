#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/svg.h"


void svg_iniciar(FILE *arquivo) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
            "width=\"100%%\" height=\"100%%\">\n");
}

void svg_iniciar_com_bbox(FILE *arquivo, double minx, double miny,
                          double maxx, double maxy, double margem) {
    assert(arquivo != NULL);

    double x = minx - margem;
    double y = miny - margem;
    double w = (maxx - minx) + 2.0 * margem;
    double h = (maxy - miny) + 2.0 * margem;

    /* Largura/altura degeneradas (bbox vazia ou com 1 ponto só) recebem um
       tamanho mínimo para o SVG não colapsar. */
    if (w <= 0.0) w = 100.0;
    if (h <= 0.0) h = 100.0;

    fprintf(arquivo,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
            "viewBox=\"%.6f %.6f %.6f %.6f\" "
            "width=\"100%%\" height=\"100%%\">\n",
            x, y, w, h);
}

/* Contexto usado pelo callback de cálculo de bbox das quadras */
typedef struct {
    double minx, miny, maxx, maxy;
    bool   tem_dado;
} CtxBBox;

static void bbox_quadra_cb(uint64_t chave, void *dado, size_t tamanho, void *contexto) {
    (void)chave;
    (void)tamanho;

    CtxBBox *ctx = (CtxBBox *)contexto;
    Quadra q = (Quadra)dado;

    double x = quadra_getX(q);
    double y = quadra_getY(q);
    double x2 = x + quadra_getW(q);
    double y2 = y + quadra_getH(q);

    if (!ctx->tem_dado) {
        ctx->minx = x;  ctx->miny = y;
        ctx->maxx = x2; ctx->maxy = y2;
        ctx->tem_dado = true;
        return;
    }

    if (x  < ctx->minx) ctx->minx = x;
    if (y  < ctx->miny) ctx->miny = y;
    if (x2 > ctx->maxx) ctx->maxx = x2;
    if (y2 > ctx->maxy) ctx->maxy = y2;
}

void svg_calcular_bbox_quadras(const Hash *tabela_quadras,
                               double *minx, double *miny,
                               double *maxx, double *maxy) {
    assert(tabela_quadras != NULL);
    assert(minx && miny && maxx && maxy);

    CtxBBox ctx = { 0.0, 0.0, 0.0, 0.0, false };
    hash_foreach(tabela_quadras, bbox_quadra_cb, &ctx);

    *minx = ctx.minx;
    *miny = ctx.miny;
    *maxx = ctx.maxx;
    *maxy = ctx.maxy;
}

void svg_fechar(FILE *arquivo) {
    assert(arquivo != NULL);
    fprintf(arquivo, "</svg>\n");
}

void svg_copiar(FILE *dst, FILE *src) {
    assert(dst != NULL && src != NULL);
    rewind(src);
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        fwrite(buf, 1, n, dst);
    }
}

/* ==========================================================================
   QUADRAS
   ========================================================================== */

void svg_desenhar_quadra(FILE *arquivo, const Quadra q) {
    assert(arquivo != NULL && q != NULL);

    double x = quadra_getX(q);
    double y = quadra_getY(q);
    double w = quadra_getW(q);
    double h = quadra_getH(q);
    const char *corp = quadra_getCorp(q);
    const char *corb = quadra_getCorb(q);
    double sw = quadra_getSw(q);

    fprintf(arquivo,
            "\t<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "fill=\"%s\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
            x, y, w, h,
            (corp && corp[0]) ? corp : "none",
            (corb && corb[0]) ? corb : "black",
            sw);

    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" font-size=\"10\" "
            "text-anchor=\"middle\" dominant-baseline=\"middle\">%s</text>\n",
            x + w / 2.0, y + h / 2.0, quadra_getCep(q));
}

typedef struct {
    FILE *arquivo;
} CtxDesenharQuadras;

static void cb_desenhar_quadra(uint64_t chave, void *dado, size_t tamanho, void *contexto) {
    (void)chave;
    (void)tamanho;
    CtxDesenharQuadras *ctx = (CtxDesenharQuadras *)contexto;
    Quadra q = (Quadra)dado; // dado ja aponta para os bytes da struct real
    svg_desenhar_quadra(ctx->arquivo, q);
}

void svg_desenhar_todas_quadras(FILE *arquivo, const Hash *tabela_quadras) {
    assert(arquivo != NULL && tabela_quadras != NULL);
    CtxDesenharQuadras ctx = { arquivo };
    hash_foreach(tabela_quadras, cb_desenhar_quadra, &ctx);
}

/* ==========================================================================
   PRIMITIVAS GENÉRICAS
   ========================================================================== */

void svg_desenhar_segmento(FILE *arquivo, double x1, double y1, double x2, double y2,
                           const char *cor, double largura) {
    assert(arquivo != NULL && cor != NULL);
    fprintf(arquivo,
            "\t<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
            x1, y1, x2, y2, cor, largura);
}

void svg_desenhar_retangulo_transparente(FILE *arquivo, double minx, double miny,
                                         double maxx, double maxy,
                                         const char *cor, double opacidade) {
    assert(arquivo != NULL && cor != NULL);

    // Normaliza para o caso de minx/miny e maxx/maxy virem trocados.
    double x = minx < maxx ? minx : maxx;
    double y = miny < maxy ? miny : maxy;
    double w = (maxx > minx ? maxx - minx : minx - maxx);
    double h = (maxy > miny ? maxy - miny : miny - maxy);

    // Bounding box degenerado (largura/altura zero, ex.: componente com um
    // unico vertice) recebe uma folga minima para permanecer visivel.
    if (w <= 0.0) { x -= 5.0; w = 10.0; }
    if (h <= 0.0) { y -= 5.0; h = 10.0; }

    fprintf(arquivo,
            "\t<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "fill=\"%s\" fill-opacity=\"%.2f\" stroke=\"%s\" stroke-width=\"1\"/>\n",
            x, y, w, h, cor, opacidade, cor);
}

/* ==========================================================================
   COMANDO @o?
   ========================================================================== */

void svg_marcar_endereco(FILE *arquivo, double x, double y, const char *id_registrador) {
    assert(arquivo != NULL && id_registrador != NULL);

    fprintf(arquivo,
            "\t<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"0\" "
            "stroke=\"red\" stroke-width=\"1.5\" stroke-dasharray=\"4,4\"/>\n",
            x, y, x);

    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"12\" fill=\"red\" font-size=\"10\" "
            "text-anchor=\"middle\">%s</text>\n",
            x, id_registrador);
}

/* ==========================================================================
   COMANDO p? — PERCURSOS ANIMADOS
   ========================================================================== */

void svg_desenhar_percurso_animado(FILE *arquivo, const double *xs, const double *ys,
                                   int n_pontos, const char *cor,
                                   const char *path_id, double duracao_s) {
    assert(arquivo != NULL && xs != NULL && ys != NULL && cor != NULL && path_id != NULL);
    assert(n_pontos >= 2);

    // Constroi o atributo 'd' do path: "M x0,y0 L x1,y1 L x2,y2 ..."
    fprintf(arquivo, "\t<path id=\"%s\" d=\"M %.2f,%.2f", path_id, xs[0], ys[0]);
    for (int i = 1; i < n_pontos; i++) {
        fprintf(arquivo, " L %.2f,%.2f", xs[i], ys[i]);
    }
    fprintf(arquivo, "\" stroke=\"%s\" stroke-width=\"3.5\" fill=\"none\" opacity=\"0.85\"/>\n", cor);

    // Figura animada percorrendo o path construido acima.
    fprintf(arquivo,
            "\t<circle r=\"5\" fill=\"%s\">\n"
            "\t\t<animateMotion dur=\"%.2fs\" repeatCount=\"indefinite\">\n"
            "\t\t\t<mpath xlink:href=\"#%s\"/>\n"
            "\t\t</animateMotion>\n"
            "\t</circle>\n",
            cor, duracao_s, path_id);
}

void svg_marcar_placa(FILE *arquivo, double x, double y, char letra, const char *cor) {
    assert(arquivo != NULL && cor != NULL);

    fprintf(arquivo,
            "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"8\" fill=\"%s\" stroke=\"black\" stroke-width=\"1\"/>\n",
            x, y, cor);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"white\" font-size=\"10\" font-weight=\"bold\" "
            "text-anchor=\"middle\" dominant-baseline=\"central\">%c</text>\n",
            x, y, letra);
}

/* ==========================================================================
   GRAFO VIÁRIO — vértices e arestas (camada base do mapa)
   ========================================================================== */

void svg_definir_marcador_seta(FILE *arquivo) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "\t<defs>\n"
            "\t  <marker id=\"seta\" markerWidth=\"6\" markerHeight=\"6\" refX=\"6\" refY=\"3\" orient=\"auto\">\n"
            "\t    <path d=\"M0,0 L0,6 L6,3 z\" fill=\"gray\"/>\n"
            "\t  </marker>\n"
            "\t</defs>\n");
}

void svg_desenhar_vertice(FILE *arquivo, double x, double y) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"3\" fill=\"orange\" stroke=\"black\" stroke-width=\"0.5\"/>\n",
            x, y);
}

void svg_desenhar_vertice_rotulado(FILE *arquivo, double x, double y, const char *id) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"3\" fill=\"orange\" stroke=\"black\" stroke-width=\"0.5\"/>\n",
            x, y);
    if (id != NULL && id[0] != '\0') {
        fprintf(arquivo,
                "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" font-size=\"4\" "
                "text-anchor=\"middle\">%s</text>\n",
                x, y - 5.0, id);
    }
}

void svg_desenhar_aresta_viaria(FILE *arquivo, double x1, double y1, double x2, double y2) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "\t<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "stroke=\"gray\" stroke-width=\"1\" marker-end=\"url(#seta)\"/>\n",
            x1, y1, x2, y2);
}

/* ==========================================================================
   WRAPPERS — nomes usados por main.c / parserGeo.c / parserQry.c
   ========================================================================== */

void svg_abrir(FILE *arquivo) {
    svg_iniciar(arquivo);
}

void svg_retangulo(FILE *arquivo, double x, double y, double w, double h,
                   const char *corp, const char *corb, double sw) {
    assert(arquivo != NULL && corp != NULL && corb != NULL);
    fprintf(arquivo,
            "\t<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "fill=\"%s\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
            x, y, w, h, corp, corb, sw);
}

void svg_linha(FILE *arquivo, double x1, double y1, double x2, double y2,
              const char *cor, double largura, bool tracejado) {
    assert(arquivo != NULL && cor != NULL);
    if (tracejado) {
        fprintf(arquivo,
                "\t<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
                "stroke=\"%s\" stroke-width=\"%.2f\" stroke-dasharray=\"4,4\"/>\n",
                x1, y1, x2, y2, cor, largura);
    } else {
        svg_desenhar_segmento(arquivo, x1, y1, x2, y2, cor, largura);
    }
}

void svg_texto(FILE *arquivo, double x, double y, const char *texto,
              const char *cor, double tamanho_fonte) {
    assert(arquivo != NULL && texto != NULL && cor != NULL);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"%s\" font-size=\"%.2f\" "
            "text-anchor=\"middle\">%s</text>\n",
            x, y, cor, tamanho_fonte, texto);
}

void svg_retangulo_transparente(FILE *arquivo, double minx, double miny,
                                double maxx, double maxy, const char *cor) {
    svg_desenhar_retangulo_transparente(arquivo, minx, miny, maxx, maxy, cor, 0.5);
}

static int svg_contador_paths_internos = 0;

void svg_caminho_animado(FILE *arquivo, const double *xs, const double *ys,
                         int n_pontos, const char *cor, double duracao_s) {
    char path_id[40];
    snprintf(path_id, sizeof(path_id), "percurso_auto_%d", svg_contador_paths_internos++);
    svg_desenhar_percurso_animado(arquivo, xs, ys, n_pontos, cor, path_id, duracao_s);
}

void svg_placa(FILE *arquivo, double x, double y, char letra) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "\t<rect x=\"%.2f\" y=\"%.2f\" width=\"14\" height=\"14\" "
            "fill=\"white\" stroke=\"black\" stroke-width=\"1\"/>\n",
            x - 7.0, y - 7.0);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" font-size=\"10\" text-anchor=\"middle\" "
            "dominant-baseline=\"middle\" font-weight=\"bold\">%c</text>\n",
            x, y, letra);
}