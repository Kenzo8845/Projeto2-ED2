#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "../include/componentesConexos.h"

typedef struct {
    int **vizinhos;
    int  *num_vizinhos;
    int  *capacidade;
} AdjND; /* adjacência não-direcionada */

static void adj_nd_destruir(AdjND *adj, int n) {
    for (int i = 0; i < n; i++) free(adj->vizinhos[i]);
    free(adj->vizinhos);
    free(adj->num_vizinhos);
    free(adj->capacidade);
}

static void adj_nd_adicionar(AdjND *adj, int u, int w) {
    for (int i = 0; i < adj->num_vizinhos[u]; i++) {
        if (adj->vizinhos[u][i] == w) return;
    }

    if (adj->num_vizinhos[u] >= adj->capacidade[u]) {
        adj->capacidade[u] = adj->capacidade[u] == 0 ? 4 : adj->capacidade[u] * 2;
        adj->vizinhos[u] = realloc(adj->vizinhos[u],
                                   sizeof(int) * (size_t)adj->capacidade[u]);
        assert(adj->vizinhos[u] != NULL);
    }
    adj->vizinhos[u][adj->num_vizinhos[u]++] = w;
}

typedef struct {
    AdjND  *adj;
    double  vl;
} CtxColeta;

static void coletar_cb(int origem, int destino, const char *nome,
                       const char *ldir, const char *lesq,
                       double cmp, double *vm, void *contexto) {
    (void)nome; (void)ldir; (void)lesq; (void)cmp;

    CtxColeta *ctx = (CtxColeta *)contexto;
    if (*vm < ctx->vl) return; 

    adj_nd_adicionar(ctx->adj, origem,  destino);
    adj_nd_adicionar(ctx->adj, destino, origem);
}

/* ==========================================================================
   BFS ITERATIVO — calcula componentes conexos
   ========================================================================== */

int componentes_conexos_calcular(Grafo *g, double vl,
                                 ComponenteCallback callback, void *contexto) {
    int n = grafo_get_num_vertices(g);
    if (n == 0) return 0;

    AdjND adj;
    adj.vizinhos     = calloc((size_t)n, sizeof(int *));
    adj.num_vizinhos = calloc((size_t)n, sizeof(int));
    adj.capacidade   = calloc((size_t)n, sizeof(int));
    assert(adj.vizinhos && adj.num_vizinhos && adj.capacidade);

    CtxColeta ctx_coleta = { &adj, vl };
    grafo_foreach_aresta(g, coletar_cb, &ctx_coleta);

    bool *visitado = calloc((size_t)n, sizeof(bool));
    int  *fila     = malloc(sizeof(int) * (size_t)n);
    assert(visitado && fila);

    int cap_comp = 16;
    double *minx = malloc(sizeof(double) * (size_t)cap_comp);
    double *miny = malloc(sizeof(double) * (size_t)cap_comp);
    double *maxx = malloc(sizeof(double) * (size_t)cap_comp);
    double *maxy = malloc(sizeof(double) * (size_t)cap_comp);
    assert(minx && miny && maxx && maxy);

    int num_comp = 0;

    for (int s = 0; s < n; s++) {
        if (visitado[s]) continue;

        int comp_idx = num_comp;

        if (comp_idx >= cap_comp) {
            cap_comp *= 2;
            minx = realloc(minx, sizeof(double) * (size_t)cap_comp);
            miny = realloc(miny, sizeof(double) * (size_t)cap_comp);
            maxx = realloc(maxx, sizeof(double) * (size_t)cap_comp);
            maxy = realloc(maxy, sizeof(double) * (size_t)cap_comp);
            assert(minx && miny && maxx && maxy);
        }

        minx[comp_idx] =  1e18;
        miny[comp_idx] =  1e18;
        maxx[comp_idx] = -1e18;
        maxy[comp_idx] = -1e18;

        visitado[s] = true;
        int ini = 0, fim = 0;
        fila[fim++] = s;

        {
            double vx = grafo_get_vertice_x(g, s);
            double vy = grafo_get_vertice_y(g, s);
            if (vx >= 0.0 && vy >= 0.0) {
                minx[comp_idx] = maxx[comp_idx] = vx;
                miny[comp_idx] = maxy[comp_idx] = vy;
            }
        }

        while (ini < fim) {
            int u = fila[ini++];

            for (int i = 0; i < adj.num_vizinhos[u]; i++) {
                int w = adj.vizinhos[u][i];
                if (visitado[w]) continue;

                visitado[w] = true;
                fila[fim++] = w;

                double wx = grafo_get_vertice_x(g, w);
                double wy = grafo_get_vertice_y(g, w);
                if (wx < 0.0 || wy < 0.0) continue;

                if (wx < minx[comp_idx]) minx[comp_idx] = wx;
                if (wx > maxx[comp_idx]) maxx[comp_idx] = wx;
                if (wy < miny[comp_idx]) miny[comp_idx] = wy;
                if (wy > maxy[comp_idx]) maxy[comp_idx] = wy;
            }
        }

        if (minx[comp_idx] > maxx[comp_idx]) {
            minx[comp_idx] = maxx[comp_idx] = 0.0;
            miny[comp_idx] = maxy[comp_idx] = 0.0;
        }

        num_comp++;
    }

    if (callback != NULL) {
        for (int c = 0; c < num_comp; c++) {
            callback(c, minx[c], miny[c], maxx[c], maxy[c], contexto);
        }
    }

    free(minx); free(miny); free(maxx); free(maxy);
    free(fila);
    free(visitado);
    adj_nd_destruir(&adj, n);

    return num_comp;
}