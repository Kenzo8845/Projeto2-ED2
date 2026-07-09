#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <assert.h>

#include "../include/dijkstra.h"

/* ==========================================================================
   MIN-HEAP (fila de prioridade) — cresce dinamicamente, sem limite fixo.
   ========================================================================== */

typedef struct {
    int    v_idx;
    double prioridade;
} HeapNo;

typedef struct {
    HeapNo *dados;
    int     tamanho;
    int     capacidade;
} MinHeap;

static MinHeap *heap_create(int capacidade_inicial) {
    if (capacidade_inicial < 1) capacidade_inicial = 1;

    MinHeap *h = malloc(sizeof(MinHeap));
    assert(h != NULL);
    h->dados = malloc(sizeof(HeapNo) * (size_t)capacidade_inicial);
    assert(h->dados != NULL);
    h->tamanho = 0;
    h->capacidade = capacidade_inicial;
    return h;
}

static void heap_destroy(MinHeap *h) {
    if (h == NULL) return;
    free(h->dados);
    free(h);
}

static void heap_garantir_capacidade(MinHeap *h) {
    if (h->tamanho < h->capacidade) return;
    h->capacidade *= 2;
    HeapNo *novo = realloc(h->dados, sizeof(HeapNo) * (size_t)h->capacidade);
    assert(novo != NULL);
    h->dados = novo;
}

static void heap_swap(MinHeap *h, int i, int j) {
    HeapNo tmp = h->dados[i];
    h->dados[i] = h->dados[j];
    h->dados[j] = tmp;
}

static void heap_push(MinHeap *h, int v_idx, double prioridade) {
    heap_garantir_capacidade(h);

    int i = h->tamanho++;
    h->dados[i].v_idx = v_idx;
    h->dados[i].prioridade = prioridade;

    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->dados[i].prioridade >= h->dados[pai].prioridade) break;
        heap_swap(h, i, pai);
        i = pai;
    }
}

// Pré-condição: h->tamanho > 0.
static HeapNo heap_pop(MinHeap *h) {
    HeapNo topo = h->dados[0];

    h->tamanho--;
    if (h->tamanho > 0) {
        h->dados[0] = h->dados[h->tamanho];

        int i = 0;
        for (;;) {
            int esq = 2 * i + 1;
            int dir = 2 * i + 2;
            int menor = i;

            if (esq < h->tamanho && h->dados[esq].prioridade < h->dados[menor].prioridade) menor = esq;
            if (dir < h->tamanho && h->dados[dir].prioridade < h->dados[menor].prioridade) menor = dir;
            if (menor == i) break;

            heap_swap(h, i, menor);
            i = menor;
        }
    }
    return topo;
}

/* ==========================================================================
   DIJKSTRA
   ========================================================================== */

typedef struct {
    int      origem_atual;
    bool     por_tempo;
    double  *distancias;
    int     *antecessores;
    MinHeap *heap;
} ContextoDijkstra;

static void avaliar_vizinho_cb(int destino, double cmp, double vm, void *contexto) {
    ContextoDijkstra *ctx = (ContextoDijkstra *)contexto;

    // vm <= 0 indicaria trecho intransitável; evita divisão por zero.
    double custo_aresta = ctx->por_tempo ? (vm > 0.0 ? cmp / vm : DBL_MAX) : cmp;
    if (custo_aresta >= DBL_MAX) return;

    double nova_dist = ctx->distancias[ctx->origem_atual] + custo_aresta;

    if (nova_dist < ctx->distancias[destino]) {
        ctx->distancias[destino] = nova_dist;
        ctx->antecessores[destino] = ctx->origem_atual;
        heap_push(ctx->heap, destino, nova_dist);
    }
}

bool dijkstra_buscar(const Grafo *g, int idx_origem, int idx_destino, bool por_tempo,
                     int *out_antecessores, double *out_distancias) {
    int num_v = grafo_get_num_vertices(g);
    if (idx_origem < 0 || idx_origem >= num_v || idx_destino < 0 || idx_destino >= num_v) {
        return false;
    }

    for (int i = 0; i < num_v; i++) {
        out_distancias[i] = DBL_MAX;
        out_antecessores[i] = -1;
    }

    MinHeap *heap = heap_create(num_v > 0 ? num_v : 1);
    bool *visitados = calloc((size_t)num_v, sizeof(bool));
    assert(visitados != NULL);

    out_distancias[idx_origem] = 0.0;
    heap_push(heap, idx_origem, 0.0);

    ContextoDijkstra ctx = {
        .origem_atual = idx_origem,
        .por_tempo = por_tempo,
        .distancias = out_distancias,
        .antecessores = out_antecessores,
        .heap = heap,
    };

    bool encontrou = false;

    while (heap->tamanho > 0) {
        HeapNo atual = heap_pop(heap);
        int u = atual.v_idx;

        if (visitados[u]) continue; // entrada obsoleta (lazy deletion)
        visitados[u] = true;

        if (u == idx_destino) {
            encontrou = true;
            break;
        }

        ctx.origem_atual = u;
        grafo_foreach_vizinho(g, u, avaliar_vizinho_cb, &ctx);
    }

    heap_destroy(heap);
    free(visitados);

    return encontrou;
}