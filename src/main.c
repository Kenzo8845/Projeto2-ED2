#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <unistd.h>

#include "../include/hash.h"
#include "../include/quadra.h"
#include "../include/grafo.h"
#include "../include/svg.h"
#include "../include/parserGeo.h"
#include "../include/parserVia.h"
#include "../include/parserQry.h"

#define SVG_MARGEM 50.0

/* =========================================================
   UTILITÁRIOS DE CAMINHO
   ========================================================= */

static void montar_caminho(char *destino, size_t tam, const char *bed, const char *arquivo) {
    if (bed != NULL) {
        snprintf(destino, tam, "%s/%s", bed, arquivo);
    } else {
        snprintf(destino, tam, "%s", arquivo);
    }
}

static void extrair_nome_base(const char *caminho, char *destino, size_t tam) {
    const char *barra = strrchr(caminho, '/');
    const char *nome = barra != NULL ? barra + 1 : caminho;
    const char *ponto = strrchr(nome, '.');
    size_t len = ponto != NULL ? (size_t)(ponto - nome) : strlen(nome);
    if (len >= tam) len = tam - 1;
    memcpy(destino, nome, len);
    destino[len] = '\0';
}

static void imprimir_uso(void) {
    fprintf(stderr, "Uso: ted [-e path] -f arq.geo [-q consulta.qry] [-v arqvias.via] -o dir\n");
}

/* =========================================================
   DESENHO 
   ========================================================= */

typedef struct {
    FILE        *svg;
    const Grafo *g;
} CtxDesenharVias;

static void cb_desenhar_aresta_via(int origem, int destino, const char *nome,
                                   const char *ldir, const char *lesq,
                                   double cmp, double *vm, void *contexto) {
    (void)nome; (void)ldir; (void)lesq; (void)cmp; (void)vm;

    CtxDesenharVias *ctx = (CtxDesenharVias *)contexto;
    double x1 = grafo_get_vertice_x(ctx->g, origem);
    double y1 = grafo_get_vertice_y(ctx->g, origem);
    double x2 = grafo_get_vertice_x(ctx->g, destino);
    double y2 = grafo_get_vertice_y(ctx->g, destino);

    svg_desenhar_aresta_viaria(ctx->svg, x1, y1, x2, y2);
}

static void desenhar_malha_viaria(FILE *svg, Grafo *g) {
    if (g == NULL || svg == NULL) return;

    svg_definir_marcador_seta(svg);

    CtxDesenharVias ctx = { svg, g };
    grafo_foreach_aresta(g, cb_desenhar_aresta_via, &ctx);

    int nv = grafo_get_num_vertices(g);
    for (int i = 0; i < nv; i++) {
        svg_desenhar_vertice_rotulado(svg, grafo_get_vertice_x(g, i), grafo_get_vertice_y(g, i), grafo_get_vertice_id(g, i));
    }
}

static void expandir_bbox_com_grafo(const Grafo *g, double *minx, double *miny,
                                    double *maxx, double *maxy, bool *tem_dado) {
    if (g == NULL) return;

    int nv = grafo_get_num_vertices(g);
    for (int i = 0; i < nv; i++) {
        double x = grafo_get_vertice_x(g, i);
        double y = grafo_get_vertice_y(g, i);

        if (!*tem_dado) {
            *minx = *maxx = x;
            *miny = *maxy = y;
            *tem_dado = true;
            continue;
        }
        if (x < *minx) *minx = x;
        if (x > *maxx) *maxx = x;
        if (y < *miny) *miny = y;
        if (y > *maxy) *maxy = y;
    }
}

/* =========================================================
   MAIN
   ========================================================= */
int main(int argc, char *argv[]) {
    char *bed = NULL;
    char *arq_geo = NULL;
    char *arq_qry = NULL;
    char *arq_via = NULL;
    char *dir_saida = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "e:f:q:v:o:")) != -1) {
        switch (opt) {
            case 'e': bed = optarg; break;
            case 'f': arq_geo = optarg; break;
            case 'q': arq_qry = optarg; break;
            case 'v': arq_via = optarg; break;
            case 'o': dir_saida = optarg; break;
            default: imprimir_uso(); return 1;
        }
    }

    if (arq_geo == NULL || dir_saida == NULL) {
        imprimir_uso();
        return 1;
    }

    char caminho_geo[512];
    montar_caminho(caminho_geo, sizeof(caminho_geo), bed, arq_geo);

    char nome_base_geo[256];
    extrair_nome_base(arq_geo, nome_base_geo, sizeof(nome_base_geo));

    /* =====================================================
       ETAPA 1 — .geo 
       ===================================================== */
    Hash *quadras = hash_create(8);

    FILE *buffer_geo = tmpfile();
    if (buffer_geo == NULL) {
        fprintf(stderr, "Erro ao criar arquivo temporario.\n");
        hash_destroy(quadras);
        return 1;
    }

    parser_geo_processar(caminho_geo, quadras, buffer_geo);

    /* Bbox inicial: so das quadras. Sera expandido abaixo se houver .via. */
    double minx = 0, miny = 0, maxx = 0, maxy = 0;
    svg_calcular_bbox_quadras(quadras, &minx, &miny, &maxx, &maxy);
    bool bbox_tem_dado = true; /* svg_calcular_bbox_quadras ja cobre o caso vazio (retorna 0s) */

    /* =====================================================
       ETAPA 2 — Se houver .via, monta o grafo.
       ===================================================== */
    Grafo *vias = grafo_create(64);
    bool tem_via = false;

    if (arq_via != NULL) {
        char caminho_via[512];
        montar_caminho(caminho_via, sizeof(caminho_via), bed, arq_via);
        tem_via = parser_via_processar(caminho_via, vias);

        if (tem_via) {
            expandir_bbox_com_grafo(vias, &minx, &miny, &maxx, &maxy, &bbox_tem_dado);
            desenhar_malha_viaria(buffer_geo, vias);
        }
    }

    /* =====================================================
       ETAPA 3 — arq.svg
       ===================================================== */
    char caminho_svg_geo[512];
    snprintf(caminho_svg_geo, sizeof(caminho_svg_geo), "%s/%s.svg", dir_saida, nome_base_geo);

    FILE *svg_geo_final = fopen(caminho_svg_geo, "w");
    if (svg_geo_final == NULL) {
        fprintf(stderr, "Erro ao criar arquivo de saida: %s\n", caminho_svg_geo);
        fclose(buffer_geo);
        grafo_destroy(vias);
        hash_destroy(quadras);
        return 1;
    }

    svg_iniciar_com_bbox(svg_geo_final, minx, miny, maxx, maxy, SVG_MARGEM);
    svg_copiar(svg_geo_final, buffer_geo);
    svg_fechar(svg_geo_final);
    fclose(svg_geo_final);
    fclose(buffer_geo);

    /* =====================================================
       ETAPA 4 — qry
       ===================================================== */
    if (arq_qry != NULL) {
        if (!tem_via) {
            fprintf(stderr,
                "Aviso: -q informado sem -v; consultas de trajeto exigem o grafo viario.\n");
        }

        char caminho_qry[512];
        montar_caminho(caminho_qry, sizeof(caminho_qry), bed, arq_qry);

        char nome_base_qry[256];
        extrair_nome_base(arq_qry, nome_base_qry, sizeof(nome_base_qry));

        char caminho_svg_final[600], caminho_txt_final[600];
        snprintf(caminho_svg_final, sizeof(caminho_svg_final), "%s/%s-%s.svg",
                 dir_saida, nome_base_geo, nome_base_qry);
        snprintf(caminho_txt_final, sizeof(caminho_txt_final), "%s/%s-%s.txt",
                 dir_saida, nome_base_geo, nome_base_qry);

        FILE *txt_final = fopen(caminho_txt_final, "w");
        FILE *buffer_qry = tmpfile();

        if (txt_final == NULL || buffer_qry == NULL) {
            fprintf(stderr, "Erro ao criar arquivos de saida para -q.\n");
        } else {
            svg_desenhar_todas_quadras(buffer_qry, quadras);
            if (tem_via) desenhar_malha_viaria(buffer_qry, vias);

            parser_qry_processar(caminho_qry, quadras, vias, txt_final, buffer_qry);

            FILE *svg_final = fopen(caminho_svg_final, "w");
            if (svg_final != NULL) {
                svg_iniciar_com_bbox(svg_final, minx, miny, maxx, maxy, SVG_MARGEM);
                svg_copiar(svg_final, buffer_qry);
                svg_fechar(svg_final);
                fclose(svg_final);
            } else {
                fprintf(stderr, "Erro ao criar arquivo: %s\n", caminho_svg_final);
            }
        }

        if (txt_final)   fclose(txt_final);
        if (buffer_qry)  fclose(buffer_qry);
    }

    grafo_destroy(vias);
    hash_destroy(quadras);

    return 0;
}