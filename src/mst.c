#include <stdlib.h>
#include <assert.h>

#include "../include/mst.h"

typedef struct {
    int *pai;
    int *rank_;
} UnionFind;

static UnionFind *uf_create(int n) {
    UnionFind *uf = malloc(sizeof(UnionFind));
    assert(uf != NULL);
    uf->pai = malloc(sizeof(int) * (size_t)n);
    uf->rank_ = calloc((size_t)n, sizeof(int));
    assert(uf->pai != NULL && uf->rank_ != NULL);
    for (int i = 0; i < n; i++) uf->pai[i] = i;
    return uf;
}

static void uf_destroy(UnionFind *uf) {
    if (uf == NULL) return;
    free(uf->pai);
    free(uf->rank_);
    free(uf);
}

static int uf_find(UnionFind *uf, int x) {
    while (uf->pai[x] != x) {
        uf->pai[x] = uf->pai[uf->pai[x]];
        x = uf->pai[x];
    }
    return x;
}

static bool uf_union(UnionFind *uf, int a, int b) {
    int ra = uf_find(uf, a);
    int rb = uf_find(uf, b);
    if (ra == rb) return false;

    if (uf->rank_[ra] < uf->rank_[rb]) {
        int tmp = ra; ra = rb; rb = tmp;
    }
    uf->pai[rb] = ra;
    if (uf->rank_[ra] == uf->rank_[rb]) uf->rank_[ra]++;
    return true;
}

typedef struct {
    int origem;
    int destino;
    double cmp;
    double *vm;
} Candidato;

typedef struct {
    Candidato *dados;
    int n;
    int capacidade;
} ListaCandidatos;

static void coletar_aresta_cb(int origem, int destino, const char *nome, const char *ldir, const char *lesq, double cmp, double *vm, void *contexto) {
    (void)nome; (void)ldir; (void)lesq;

    ListaCandidatos *lista = (ListaCandidatos *)contexto;
    if (lista->n >= lista->capacidade) {
        lista->capacidade *= 2;
        lista->dados = realloc(lista->dados, sizeof(Candidato) * (size_t)lista->capacidade);
        assert(lista->dados != NULL);
    }

    Candidato *c = &lista->dados[lista->n++];
    c->origem = origem;
    c->destino = destino;
    c->cmp = cmp;
    c->vm = vm;
}

static int comparar_por_cmp(const void *a, const void *b) {
    const Candidato *ca = (const Candidato *)a;
    const Candidato *cb = (const Candidato *)b;
    if (ca->cmp < cb->cmp) return -1;
    if (ca->cmp > cb->cmp) return 1;
    return 0;
}

int mst_ampliar_vias(Grafo *g, double vl, MstArestaCallback callback, void *contexto) {
    int n = grafo_get_num_vertices(g);
    if (n == 0) return 0;

    ListaCandidatos lista;
    lista.capacidade = 16;
    lista.n = 0;
    lista.dados = malloc(sizeof(Candidato) * (size_t)lista.capacidade);
    assert(lista.dados != NULL);

    grafo_foreach_aresta(g, coletar_aresta_cb, &lista);

    if (lista.n == 0) {
        free(lista.dados);
        return 0;
    }

    qsort(lista.dados, (size_t)lista.n, sizeof(Candidato), comparar_por_cmp);

    UnionFind *uf = uf_create(n);
    int ampliadas = 0;

    for (int i = 0; i < lista.n; i++) {
        Candidato *c = &lista.dados[i];
        if (!uf_union(uf, c->origem, c->destino)) continue;

        if (*c->vm < vl) {
            double vm_nova = *c->vm * 1.5;
            *c->vm = vm_nova;
            ampliadas++;
            if (callback != NULL) {
                callback(c->origem, c->destino, c->cmp, vm_nova, contexto);
            }
        }
    }

    uf_destroy(uf);
    free(lista.dados);

    return ampliadas;
}