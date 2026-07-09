#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#include "../include/grafo.h"
#include "../include/hash.h"

typedef struct Aresta {
    int destino;
    char *nome;
    char *ldir;
    char *lesq;
    double cmp;
    double vm;
    struct Aresta *proxima;
} Aresta;

typedef struct {
    char   *id;
    double  x;
    double  y;
    Aresta *primeira_aresta;
    int     num_arestas_saida;
} Vertice;

struct Grafo {
    Vertice *vertices;
    int num_vertices;
    int capacidade;
    Hash *mapa_ids;
};

static char *duplicar_string(const char *origem) {
    if (origem == NULL) origem = "";
    size_t len = strlen(origem) + 1;
    char *copia = malloc(len);
    assert(copia != NULL);
    memcpy(copia, origem, len);
    return copia;
}

/* ==========================================================================
   CONSTRUCTOR / DESTRUCTOR
   ========================================================================== */

Grafo *grafo_create(int capacidade_inicial) {
    if (capacidade_inicial < 1) capacidade_inicial = 1;

    Grafo *g = malloc(sizeof(Grafo));
    assert(g != NULL);

    g->vertices = malloc(sizeof(Vertice) * (size_t)capacidade_inicial);
    assert(g->vertices != NULL);

    g->num_vertices = 0;
    g->capacidade = capacidade_inicial;
    g->mapa_ids = hash_create(4);
    assert(g->mapa_ids != NULL);

    return g;
}

void grafo_destroy(Grafo *g) {
    if (g == NULL) return;

    for (int i = 0; i < g->num_vertices; i++) {
        free(g->vertices[i].id);

        Aresta *atual = g->vertices[i].primeira_aresta;
        while (atual != NULL) {
            Aresta *prox = atual->proxima;
            free(atual->nome);
            free(atual->ldir);
            free(atual->lesq);
            free(atual);
            atual = prox;
        }
    }

    free(g->vertices);
    hash_destroy(g->mapa_ids);
    free(g);
}

/* ==========================================================================
   VÉRTICES
   ========================================================================== */

int grafo_find_vertice(const Grafo *g, const char *id) {
    if (g == NULL || id == NULL) return -1;

    uint64_t chave = hash_string_djb2(id);
    size_t tamanho;
    int *encontrado = (int *)hash_search(g->mapa_ids, chave, &tamanho);

    if (encontrado == NULL) return -1;

    int idx = *encontrado;
    free(encontrado);
    return idx;
}

int grafo_add_vertice(Grafo *g, const char *id, double x, double y) {
    assert(g != NULL && id != NULL);

    int idx_existente = grafo_find_vertice(g, id);
    if (idx_existente != -1) return idx_existente;

    if (g->num_vertices >= g->capacidade) {
        g->capacidade *= 2;
        Vertice *novo_array = realloc(g->vertices, sizeof(Vertice) * (size_t)g->capacidade);
        assert(novo_array != NULL);
        g->vertices = novo_array;
    }

    int novo_idx = g->num_vertices;

    g->vertices[novo_idx].id = duplicar_string(id);
    g->vertices[novo_idx].x = x;
    g->vertices[novo_idx].y = y;
    g->vertices[novo_idx].primeira_aresta = NULL;
    g->vertices[novo_idx].num_arestas_saida = 0;

    uint64_t chave = hash_string_djb2(id);
    hash_insert(g->mapa_ids, chave, &novo_idx, sizeof(int));

    g->num_vertices++;
    return novo_idx;
}

int grafo_get_num_vertices(const Grafo *g) {
    return g ? g->num_vertices : 0;
}

const char *grafo_get_vertice_id(const Grafo *g, int idx) {
    if (g == NULL || idx < 0 || idx >= g->num_vertices) return NULL;
    return g->vertices[idx].id;
}

double grafo_get_vertice_x(const Grafo *g, int idx) {
    if (g == NULL || idx < 0 || idx >= g->num_vertices) return 0.0;
    return g->vertices[idx].x;
}

double grafo_get_vertice_y(const Grafo *g, int idx) {
    if (g == NULL || idx < 0 || idx >= g->num_vertices) return 0.0;
    return g->vertices[idx].y;
}

int grafo_procurar_vertice_proximo(const Grafo *g, double x, double y) {
    if (g == NULL || g->num_vertices == 0) return -1;

    int melhor_idx = -1;
    double melhor_dist2 = DBL_MAX;

    for (int i = 0; i < g->num_vertices; i++) {
        double dx = g->vertices[i].x - x;
        double dy = g->vertices[i].y - y;
        double dist2 = dx * dx + dy * dy;
        if (dist2 < melhor_dist2) {
            melhor_dist2 = dist2;
            melhor_idx = i;
        }
    }
    return melhor_idx;
}

/* ==========================================================================
   ARESTAS
   ========================================================================== */

bool grafo_add_aresta(Grafo *g, const char *id_origem, const char *id_destino,
                      const char *nome, const char *ldir, const char *lesq,
                      double cmp, double vm) {
    if (g == NULL) return false;

    int idx_origem  = grafo_find_vertice(g, id_origem);
    int idx_destino = grafo_find_vertice(g, id_destino);

    if (idx_origem == -1 || idx_destino == -1) return false;

    Aresta *nova = malloc(sizeof(Aresta));
    assert(nova != NULL);

    nova->destino = idx_destino;
    nova->nome = duplicar_string(nome);
    nova->ldir = duplicar_string(ldir);
    nova->lesq = duplicar_string(lesq);
    nova->cmp = cmp;
    nova->vm  = vm;

    nova->proxima = g->vertices[idx_origem].primeira_aresta;
    g->vertices[idx_origem].primeira_aresta = nova;
    g->vertices[idx_origem].num_arestas_saida++;

    return true;
}

int grafo_get_num_arestas_saida(const Grafo *g, int idx) {
    if (g == NULL || idx < 0 || idx >= g->num_vertices) return 0;
    return g->vertices[idx].num_arestas_saida;
}

/* ==========================================================================
   ITERAÇÃO
   ========================================================================== */

void grafo_foreach_vizinho(const Grafo *g, int origem, GrafoVizinhoCallback callback, void *contexto) {
    if (g == NULL || origem < 0 || origem >= g->num_vertices || callback == NULL) return;

    for (Aresta *a = g->vertices[origem].primeira_aresta; a != NULL; a = a->proxima) {
        callback(a->destino, a->cmp, a->vm, contexto);
    }
}

void grafo_foreach_aresta(Grafo *g, GrafoArestaCallback callback, void *contexto) {
    if (g == NULL || callback == NULL) return;

    for (int i = 0; i < g->num_vertices; i++) {
        for (Aresta *a = g->vertices[i].primeira_aresta; a != NULL; a = a->proxima) {
            callback(i, a->destino, a->nome, a->ldir, a->lesq, a->cmp, &a->vm, contexto);
        }
    }
}

/* ==========================================================================
   ATUALIZAÇÃO DE VELOCIDADE POR REGIÃO (comando mvm)
   ========================================================================== */

static bool ponto_dentro_regiao(double px, double py, double xmin, double ymin, double xmax, double ymax) {
    return px >= xmin && px <= xmax && py >= ymin && py <= ymax;
}

int grafo_atualizar_velocidade_regiao(Grafo *g, double v, double x, double y, double w, double h) {
    if (g == NULL) return 0;

    // Robusto a w/h negativos: normaliza para (xmin,ymin)-(xmax,ymax).
    double xmin = (w >= 0) ? x : x + w;
    double xmax = (w >= 0) ? x + w : x;
    double ymin = (h >= 0) ? y : y + h;
    double ymax = (h >= 0) ? y + h : y;

    int atualizadas = 0;

    for (int i = 0; i < g->num_vertices; i++) {
        double ox = g->vertices[i].x, oy = g->vertices[i].y;
        bool origem_dentro = ponto_dentro_regiao(ox, oy, xmin, ymin, xmax, ymax);

        for (Aresta *a = g->vertices[i].primeira_aresta; a != NULL; a = a->proxima) {
            double dx = g->vertices[a->destino].x, dy = g->vertices[a->destino].y;
            bool destino_dentro = ponto_dentro_regiao(dx, dy, xmin, ymin, xmax, ymax);
            if (origem_dentro || destino_dentro) {
                a->vm = v;
                atualizadas++;
            }
        }
    }
    return atualizadas;
}