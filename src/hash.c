#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/hash.h"

/* ==========================================================================
   ESTRUTURAS INTERNAS
   ========================================================================== */

// Um registro guarda sua própria cópia dos dados.
typedef struct {
    uint64_t key;
    void    *data;
    size_t   size;
} Record;

// Bucket: array dinâmico de registros + profundidade local.
typedef struct Bucket {
    int     local_depth;
    int     count;      // registros válidos atualmente no bucket
    int     capacity;    // capacidade máxima (== capacidade_bucket da hash)
    Record *records;    // array de tamanho 'capacity'
} Bucket;

// Estrutura principal — opaca para o usuário (definida apenas aqui).
struct Hash {
    int      global_depth;
    int      dir_size;        // 2^global_depth
    Bucket **directory;       // directory[i] = ponteiro para o bucket
    int      bucket_capacity; // capacidade máxima de registros por bucket
};

/* ==========================================================================
   FUNÇÕES AUXILIARES INTERNAS
   ========================================================================== */

// Retorna os 'depth' bits menos significativos de key (índice no diretório).
static int hash_index(uint64_t key, int depth) {
    if (depth <= 0) return 0;
    return (int)(key & ((1ULL << depth) - 1));
}

static Bucket *bucket_create(int local_depth, int capacity) {
    Bucket *b = malloc(sizeof(Bucket));
    assert(b != NULL);
    b->local_depth = local_depth;
    b->count = 0;
    b->capacity = capacity;
    b->records = calloc((size_t)capacity, sizeof(Record));
    assert(b->records != NULL);
    return b;
}

static void bucket_destroy(Bucket *b) {
    if (b == NULL) return;
    for (int i = 0; i < b->count; i++) {
        free(b->records[i].data);
    }
    free(b->records);
    free(b);
}

static bool insert_into_bucket(Hash *h, Bucket *b, uint64_t key, const void *data, size_t size);

// Divide um bucket cheio em dois, redistribuindo seus registros.
static void split_bucket(Hash *h, Bucket *old_bucket) {
    int old_depth = old_bucket->local_depth;

    // Se a profundidade local já atingiu a global, dobra o diretório.
    if (old_depth == h->global_depth) {
        int new_size = h->dir_size * 2;
        Bucket **new_dir = realloc(h->directory, (size_t)new_size * sizeof(Bucket *));
        assert(new_dir != NULL);
        h->directory = new_dir;
        for (int i = 0; i < h->dir_size; i++) {
            h->directory[h->dir_size + i] = h->directory[i];
        }
        h->dir_size = new_size;
        h->global_depth++;
    }

    Bucket *new_bucket = bucket_create(old_depth + 1, h->bucket_capacity);
    old_bucket->local_depth = old_depth + 1;

    // Redireciona as entradas do diretório que devem apontar para o novo bucket.
    for (int i = 0; i < h->dir_size; i++) {
        if (h->directory[i] == old_bucket && ((i >> old_depth) & 1)) {
            h->directory[i] = new_bucket;
        }
    }

    // Redistribui os registros do bucket antigo entre ele mesmo e o novo.
    Record *old_records = old_bucket->records;
    int old_count = old_bucket->count;

    old_bucket->records = calloc((size_t)old_bucket->capacity, sizeof(Record));
    assert(old_bucket->records != NULL);
    old_bucket->count = 0;

    for (int i = 0; i < old_count; i++) {
        int idx = hash_index(old_records[i].key, h->global_depth);
        Bucket *destino = h->directory[idx];
        // Os dados já são uma cópia própria do registro; repassamos o ponteiro
        // (sem copiar de novo) para o bucket de destino.
        Record *slot = &destino->records[destino->count++];
        slot->key  = old_records[i].key;
        slot->data = old_records[i].data;
        slot->size = old_records[i].size;
    }
    free(old_records);
}

static bool insert_into_bucket(Hash *h, Bucket *b, uint64_t key, const void *data, size_t size) {
    if (b->count >= b->capacity) {
        split_bucket(h, b);
        int idx = hash_index(key, h->global_depth);
        return insert_into_bucket(h, h->directory[idx], key, data, size);
    }

    void *copia = malloc(size);
    assert(copia != NULL);
    memcpy(copia, data, size);

    Record *slot = &b->records[b->count++];
    slot->key  = key;
    slot->data = copia;
    slot->size = size;
    return true;
}

/* ==========================================================================
   FUNÇÕES PÚBLICAS — CICLO DE VIDA
   ========================================================================== */

Hash *hash_create(int capacidade_bucket) {
    assert(capacidade_bucket >= 1);

    Hash *h = malloc(sizeof(Hash));
    assert(h != NULL);

    h->global_depth = 1;
    h->dir_size = 2;
    h->bucket_capacity = capacidade_bucket;

    h->directory = malloc((size_t)h->dir_size * sizeof(Bucket *));
    assert(h->directory != NULL);

    h->directory[0] = bucket_create(1, capacidade_bucket);
    h->directory[1] = bucket_create(1, capacidade_bucket);

    return h;
}

void hash_destroy(Hash *h) {
    if (h == NULL) return;

    // Cada bucket pode estar referenciado por múltiplas entradas do 
    // diretório; marcamos os já destruídos para não liberar duas vezes.
    Bucket **destruidos = calloc((size_t)h->dir_size, sizeof(Bucket *));
    assert(destruidos != NULL);
    int n_destruidos = 0;

    for (int i = 0; i < h->dir_size; i++) {
        Bucket *b = h->directory[i];
        bool ja_destruido = false;
        for (int j = 0; j < n_destruidos; j++) {
            if (destruidos[j] == b) { ja_destruido = true; break; }
        }
        if (!ja_destruido) {
            destruidos[n_destruidos++] = b;
            bucket_destroy(b);
        }
    }

    free(destruidos);
    free(h->directory);
    free(h);
}

/* ==========================================================================
   FUNÇÕES PÚBLICAS — OPERAÇÕES PRINCIPAIS
   ========================================================================== */

bool hash_insert(Hash *h, uint64_t chave, const void *dado, size_t tamanho) {
    assert(h != NULL);
    assert(dado != NULL);
    assert(tamanho > 0);

    int idx = hash_index(chave, h->global_depth);
    return insert_into_bucket(h, h->directory[idx], chave, dado, tamanho);
}

void *hash_search(const Hash *h, uint64_t chave, size_t *tamanho) {
    assert(h != NULL);

    int idx = hash_index(chave, h->global_depth);
    Bucket *b = h->directory[idx];

    for (int i = 0; i < b->count; i++) {
        if (b->records[i].key == chave) {
            void *copia = malloc(b->records[i].size);
            assert(copia != NULL);
            memcpy(copia, b->records[i].data, b->records[i].size);
            if (tamanho) *tamanho = b->records[i].size;
            return copia;
        }
    }
    return NULL;
}

void *hash_remove(Hash *h, uint64_t chave, size_t *tamanho) {
    assert(h != NULL);

    int idx = hash_index(chave, h->global_depth);
    Bucket *b = h->directory[idx];

    for (int i = 0; i < b->count; i++) {
        if (b->records[i].key == chave) {
            void *dado = b->records[i].data;
            if (tamanho) *tamanho = b->records[i].size;

            // Compacta: move o último registro para o lugar do removido.
            b->records[i] = b->records[b->count - 1];
            b->count--;

            return dado; // o chamador assume a posse e deve liberar
        }
    }
    if (tamanho) *tamanho = 0;
    return NULL;
}

/* ==========================================================================
   FUNÇÕES PÚBLICAS — INTROSPECÇÃO / ITERAÇÃO
   ========================================================================== */

int hash_get_num_buckets(const Hash *h) {
    assert(h != NULL);

    Bucket **vistos = calloc((size_t)h->dir_size, sizeof(Bucket *));
    assert(vistos != NULL);
    int n_vistos = 0;

    for (int i = 0; i < h->dir_size; i++) {
        Bucket *b = h->directory[i];
        bool ja_visto = false;
        for (int j = 0; j < n_vistos; j++) {
            if (vistos[j] == b) { ja_visto = true; break; }
        }
        if (!ja_visto) vistos[n_vistos++] = b;
    }

    free(vistos);
    return n_vistos;
}

int hash_get_global_depth(const Hash *h) {
    assert(h != NULL);
    return h->global_depth;
}

void hash_foreach(const Hash *h, HashCallback callback, void *contexto) {
    assert(h != NULL);
    assert(callback != NULL);

    Bucket **vistos = calloc((size_t)h->dir_size, sizeof(Bucket *));
    assert(vistos != NULL);
    int n_vistos = 0;

    for (int i = 0; i < h->dir_size; i++) {
        Bucket *b = h->directory[i];
        bool ja_visto = false;
        for (int j = 0; j < n_vistos; j++) {
            if (vistos[j] == b) { ja_visto = true; break; }
        }
        if (ja_visto) continue;
        vistos[n_vistos++] = b;

        for (int r = 0; r < b->count; r++) {
            callback(b->records[r].key, b->records[r].data, b->records[r].size, contexto);
        }
    }

    free(vistos);
}

void hash_dump(const Hash *h, FILE *saida) {
    assert(h != NULL);
    assert(saida != NULL);

    fprintf(saida, "=== HASH EXTENSIVEL (memoria) ===\n");
    fprintf(saida, "Profundidade global : %d\n", h->global_depth);
    fprintf(saida, "Tamanho diretorio   : %d\n", h->dir_size);
    fprintf(saida, "Num. de buckets     : %d\n", hash_get_num_buckets(h));
    fprintf(saida, "Capacidade/bucket   : %d\n\n", h->bucket_capacity);

    fprintf(saida, "--- DIRETORIO ---\n");
    for (int i = 0; i < h->dir_size; i++) {
        fprintf(saida, "  dir[%3d] -> bucket %p\n", i, (void *)h->directory[i]);
    }
    fprintf(saida, "\n--- BUCKETS ---\n");

    Bucket **vistos = calloc((size_t)h->dir_size, sizeof(Bucket *));
    assert(vistos != NULL);
    int n_vistos = 0;

    for (int i = 0; i < h->dir_size; i++) {
        Bucket *b = h->directory[i];
        bool ja_visto = false;
        for (int j = 0; j < n_vistos; j++) {
            if (vistos[j] == b) { ja_visto = true; break; }
        }
        if (ja_visto) continue;
        vistos[n_vistos++] = b;

        fprintf(saida, "Bucket %p | depth_local=%d | registros=%d/%d\n",
                (void *)b, b->local_depth, b->count, b->capacity);
        for (int r = 0; r < b->count; r++) {
            fprintf(saida, "  [%d] chave=%" PRIu64 "  tamanho=%zu B\n",
                    r, b->records[r].key, b->records[r].size);
        }
    }
    fprintf(saida, "=== FIM DO DUMP ===\n");

    free(vistos);
}

/* ==========================================================================
   UTILITÁRIO DE HASHING
   ========================================================================== */

uint64_t hash_string_djb2(const char *str) {
    assert(str != NULL);
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}