#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "../include/parserQry.h"
#include "../include/quadra.h"
#include "../include/dijkstra.h"
#include "../include/svg.h"
#include "../include/componentesConexos.h"
#include "../include/mst.h"

typedef struct {
    char id[32];
    double x;
    double y;
} Registrador;

static Registrador banco_regs[128];
static int qtd_regs = 0;

static bool buscar_registrador(const char *id, double *x, double *y) {
    for (int i = 0; i < qtd_regs; i++) {
        if (strcmp(banco_regs[i].id, id) == 0) {
            *x = banco_regs[i].x;
            *y = banco_regs[i].y;
            return true;
        }
    }
    return false;
}

static void calcular_coordenada_endereco(Quadra q, char face, double num, double *out_x, double *out_y) {
    double qx = quadra_getX(q);
    double qy = quadra_getY(q);
    double qw = quadra_getW(q);
    double qh = quadra_getH(q);

    if (face == 'N') {
        *out_x = qx + num;
        *out_y = qy + qh; // Base + Altura
    } else if (face == 'S') {
        *out_x = qx + num;
        *out_y = qy;      // Base
    } else if (face == 'L' || face == 'E') {
        *out_x = qx + qw; // Direita (Largura)
        *out_y = qy + num;
    } else if (face == 'O' || face == 'W') {
        *out_x = qx;      // Esquerda
        *out_y = qy + num;
    } else {
        *out_x = qx; // Padrão caso venha lixo
        *out_y = qy;
    }
}

static void reportar_e_desenhar_trajeto(const Grafo *g, const int *antecessores, const double *distancias, int idx_destino, FILE *txt, FILE *svg, const char *cor) {
    int num_v = grafo_get_num_vertices(g);
    int *indices = malloc(sizeof(int) * (size_t)num_v);
    assert(indices != NULL);

    int n = 0;
    int atual = idx_destino;
    while (atual != -1) {
        indices[n++] = atual;
        atual = antecessores[atual];
    }

    for (int i = 0; i < n / 2; i++) {
        int tmp = indices[i];
        indices[i] = indices[n - 1 - i];
        indices[n - 1 - i] = tmp;
    }

    double *xs = malloc(sizeof(double) * (size_t)n);
    double *ys = malloc(sizeof(double) * (size_t)n);
    assert(xs != NULL && ys != NULL);

    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        xs[i] = grafo_get_vertice_x(g, idx);
        ys[i] = grafo_get_vertice_y(g, idx);
        fprintf(txt, "Siga para o cruzamento %s (%.2f, %.2f) - Acumulado: %.2f\n",
                grafo_get_vertice_id(g, idx), xs[i], ys[i], distancias[idx]);
    }

    svg_caminho_animado(svg, xs, ys, n, cor, 6.0);

    free(indices);
    free(xs);
    free(ys);
}

typedef struct {
    FILE *svg;
    int count;
} ContextoRegs;

static void desenhar_componente_cb(int comp_idx, double xmin, double ymin, double xmax, double ymax, void *contexto) {
    static const char *cores[] = { "lightgreen", "lightblue", "lightpink", "lightyellow", "lightcoral", "lightgray", "plum", "khaki" };
    ContextoRegs *ctx = (ContextoRegs *)contexto;
    const char *cor = cores[comp_idx % (int)(sizeof(cores) / sizeof(cores[0]))];
    svg_retangulo_transparente(ctx->svg, xmin, ymin, xmax, ymax, cor);
    ctx->count++;
}

typedef struct {
    const Grafo *g;
    FILE *svg;
    int count;
} ContextoExp;

static void desenhar_aresta_mst_cb(int origem, int destino, double cmp, double vm_nova, void *contexto) {
    (void)cmp; (void)vm_nova;

    ContextoExp *ctx = (ContextoExp *)contexto;
    double x1 = grafo_get_vertice_x(ctx->g, origem);
    double y1 = grafo_get_vertice_y(ctx->g, origem);
    double x2 = grafo_get_vertice_x(ctx->g, destino);
    double y2 = grafo_get_vertice_y(ctx->g, destino);

    svg_linha(ctx->svg, x1, y1, x2, y2, "red", 4.0, false);
    ctx->count++;
}

void parser_qry_processar(const char *caminho_qry, Hash *quadras, Grafo *vias, FILE *txt_saida, FILE *svg_saida) {
    FILE *arquivo = fopen(caminho_qry, "r");
    if (arquivo == NULL) {
        printf("Aviso: Nao foi possivel abrir o arquivo de consultas: %s\n", caminho_qry);
        return;
    }

    char comando[32];
    int num_vertices = grafo_get_num_vertices(vias);

    int *antecessores = malloc(sizeof(int) * (size_t)num_vertices);
    double *distancias = malloc(sizeof(double) * (size_t)num_vertices);
    assert(antecessores != NULL && distancias != NULL);

    while (fscanf(arquivo, "%s", comando) != EOF) {

        if (strcmp(comando, "@o?") == 0) {
            char reg_id[32], cep[64];
            char face;
            double num;

            if (fscanf(arquivo, "%s %s %c %lf", reg_id, cep, &face, &num) == 4) {
                uint64_t chave_cep = hash_string_djb2(cep);
                size_t tamanho;
                Quadra q_aux = (Quadra)hash_search(quadras, chave_cep, &tamanho);

                if (q_aux != NULL) {
                    double qx = quadra_getX(q_aux);
                    double qy = quadra_getY(q_aux);
                    double qw = quadra_getW(q_aux);
                    double qh = quadra_getH(q_aux);

                    double addr_x = qx, addr_y = qy;
                    switch (face) {
                       case 'S': addr_x = qx + num; addr_y = qy + qh;  break;
                        case 'N': addr_x = qx + num; addr_y = qy; break;
                        case 'L': addr_x = qx + qw; addr_y = qy + num; break; 
                        case 'O': addr_x = qx; addr_y = qy + num; break; 
                        default: break;
                    }

                    quadra_destructor(q_aux);

                    if (qtd_regs < 128) {
                        strcpy(banco_regs[qtd_regs].id, reg_id);
                        banco_regs[qtd_regs].x = addr_x;
                        banco_regs[qtd_regs].y = addr_y;
                        qtd_regs++;
                    }

                    fprintf(txt_saida, "[@] @o? %s %s %c %.2f -> Coordenada: X=%.2f, Y=%.2f\n",
                            reg_id, cep, face, num, addr_x, addr_y);
                            svg_linha(svg_saida, addr_x, addr_y, addr_x, 0.0, "red", 1.0, true);
                            svg_texto(svg_saida, addr_x, -5.0, reg_id, "red", 10.0);
                } else {
                    fprintf(txt_saida, "[@] @o? Erro: CEP %s nao encontrado.\n", cep);
                }
            }
        }

        else if (strcmp(comando, "p?") == 0) {
            char reg1[32], reg2[32], cc[64], cr[64];

            if (fscanf(arquivo, "%s %s %s %s", reg1, reg2, cc, cr) == 4) {
                fprintf(txt_saida, "\n--- CONSULTA DE TRAJETO (p? %s -> %s) ---\n", reg1, reg2);

                double x_orig, y_orig, x_dest, y_dest;
                if (!buscar_registrador(reg1, &x_orig, &y_orig) || !buscar_registrador(reg2, &x_dest, &y_dest)) {
                    fprintf(txt_saida, "Erro: Registradores invalidos ou sem coordenadas.\n");
                    continue;
                }

                int idx_origem = grafo_procurar_vertice_proximo(vias, x_orig, y_orig);
                int idx_destino = grafo_procurar_vertice_proximo(vias, x_dest, y_dest);

                if (idx_origem == -1 || idx_destino == -1) {
                    fprintf(txt_saida, "Caso destino inacessivel (cruzamento nao encontrado).\n");
                    continue;
                }

                fprintf(txt_saida, "\n[Percurso Mais Curto - Cor: %s]\n", cc);
                if (dijkstra_buscar(vias, idx_origem, idx_destino, false, antecessores, distancias)) {
                    reportar_e_desenhar_trajeto(vias, antecessores, distancias, idx_destino, txt_saida, svg_saida, cc);
                } else {
                    fprintf(txt_saida, "Caso destino inacessivel.\n");
                }

                fprintf(txt_saida, "\n[Percurso Mais Rapido - Cor: %s]\n", cr);
                if (dijkstra_buscar(vias, idx_origem, idx_destino, true, antecessores, distancias)) {
                    reportar_e_desenhar_trajeto(vias, antecessores, distancias, idx_destino, txt_saida, svg_saida, cr);
                } else {
                    fprintf(txt_saida, "Caso destino inacessivel.\n");
                }

                double xo = grafo_get_vertice_x(vias, idx_origem);
                double yo = grafo_get_vertice_y(vias, idx_origem);
                double xd = grafo_get_vertice_x(vias, idx_destino);
                double yd = grafo_get_vertice_y(vias, idx_destino);

                svg_placa(svg_saida, xo, yo, 'I');
                svg_placa(svg_saida, xd, yd, 'F');
            }
        }

        else if (strcmp(comando, "mvm") == 0) {
            double v, x, y, w, h;
            if (fscanf(arquivo, "%lf %lf %lf %lf %lf", &v, &x, &y, &w, &h) == 5) {
                int atualizadas = grafo_atualizar_velocidade_regiao(vias, v, x, y, w, h);
                fprintf(txt_saida, "[*] mvm: %d arestas atualizadas para velocidade media %.2f\n", atualizadas, v);
            }
        }

        else if (strcmp(comando, "regs") == 0) {
            double vl;
            if (fscanf(arquivo, "%lf", &vl) == 1) {
                ContextoRegs ctx = { svg_saida, 0 };
                int num_comp = componentes_conexos_calcular(vias, vl, desenhar_componente_cb, &ctx);
                fprintf(txt_saida, "[*] regs: numero de componentes conexos = %d\n", num_comp);
            }
        }

        else if (strcmp(comando, "exp") == 0) {
            double vl;
            if (fscanf(arquivo, "%lf", &vl) == 1) {
                ContextoExp ctx = { vias, svg_saida, 0 };
                int ampliadas = mst_ampliar_vias(vias, vl, desenhar_aresta_mst_cb, &ctx);
                fprintf(txt_saida, "[*] exp: %d trechos ampliados (velocidade media +50%%)\n", ampliadas);
            }
        }

        else {
            char c;
            while ((c = fgetc(arquivo)) != '\n' && c != EOF);
        }
    }

    free(antecessores);
    free(distancias);
    fclose(arquivo);
}