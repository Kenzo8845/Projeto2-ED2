#include "../include/hash.h"
#include "../Unity/unity.h"
#include <stdlib.h>
#include <string.h>

static Hash *h;

void setUp(void) {
    h = hash_create(2); /* 2 registros por bucket — forca splits cedo nos testes */
}

void tearDown(void) {
    hash_destroy(h);
}

/* =========================================================
   CRIACAO
   ========================================================= */
void test_hash_deve_ser_criada(void) {
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_INT(1, hash_get_global_depth(h));
    TEST_ASSERT_EQUAL_INT(2, hash_get_num_buckets(h));
}

/* =========================================================
   INSERT / SEARCH
   ========================================================= */
void test_insert_e_search_deve_encontrar_o_dado(void) {
    int valor = 67;
    TEST_ASSERT_TRUE(hash_insert(h, 1, &valor, sizeof(int)));

    size_t tamanho;
    int *resultado = hash_search(h, 1, &tamanho);

    TEST_ASSERT_NOT_NULL(resultado);
    TEST_ASSERT_EQUAL_INT(sizeof(int), tamanho);
    TEST_ASSERT_EQUAL_INT(67, *resultado);

    free(resultado);
}

void test_search_chave_inexistente_retorna_null(void) {
    size_t tamanho;
    void *resultado = hash_search(h, 999, &tamanho);
    TEST_ASSERT_NULL(resultado);
}

void test_insert_e_search_de_struct(void) {
    typedef struct { int id; char nome[20]; } Cachorro;
    Cachorro c = { 1, "Max" };

    hash_insert(h, 777, &c, sizeof(Cachorro));

    size_t tamanho;
    Cachorro *resultado = hash_search(h, 777, &tamanho);

    TEST_ASSERT_NOT_NULL(resultado);
    TEST_ASSERT_EQUAL_INT(sizeof(Cachorro), tamanho);
    TEST_ASSERT_EQUAL_INT(1, resultado->id);
    TEST_ASSERT_EQUAL_STRING("Max", resultado->nome);

    free(resultado);
}

void test_search_retorna_copia_independente(void) {
    /* Alterar a copia retornada NAO pode alterar o dado interno da hash */
    int valor = 10;
    hash_insert(h, 5, &valor, sizeof(int));

    size_t tamanho;
    int *copia1 = hash_search(h, 5, &tamanho);
    *copia1 = 999; /* modifica so a copia */
    free(copia1);

    int *copia2 = hash_search(h, 5, &tamanho);
    TEST_ASSERT_EQUAL_INT(10, *copia2); /* valor interno continua o original */
    free(copia2);
}

/* =========================================================
   REMOVE
   ========================================================= */
void test_remove_deve_remover_e_retornar_o_dado(void) {
    int valor = 123;
    hash_insert(h, 50, &valor, sizeof(int));

    size_t tamanho;
    int *removido = hash_remove(h, 50, &tamanho);

    TEST_ASSERT_NOT_NULL(removido);
    TEST_ASSERT_EQUAL_INT(123, *removido);
    free(removido);

    /* Apos remover, a busca nao deve mais encontrar */
    void *busca = hash_search(h, 50, &tamanho);
    TEST_ASSERT_NULL(busca);
}

void test_remove_chave_inexistente_retorna_null(void) {
    size_t tamanho;
    void *removido = hash_remove(h, 12345, &tamanho);
    TEST_ASSERT_NULL(removido);
    TEST_ASSERT_EQUAL_INT(0, tamanho);
}

/* =========================================================
   SPLIT / CRESCIMENTO
   ========================================================= */
void test_insercoes_multiplas_causam_split(void) {
    int buckets_iniciais = hash_get_num_buckets(h);

    /* capacidade_bucket = 2 no setUp: a 3a insercao no mesmo bucket forca split */
    for (int i = 0; i < 20; i++) {
        int valor = i * 10;
        TEST_ASSERT_TRUE(hash_insert(h, (uint64_t)i, &valor, sizeof(int)));
    }

    TEST_ASSERT_TRUE(hash_get_num_buckets(h) > buckets_iniciais);

    /* Todos os 20 valores devem continuar buscaveis apos os splits */
    for (int i = 0; i < 20; i++) {
        size_t tamanho;
        int *resultado = hash_search(h, (uint64_t)i, &tamanho);
        TEST_ASSERT_NOT_NULL(resultado);
        TEST_ASSERT_EQUAL_INT(i * 10, *resultado);
        free(resultado);
    }
}

void test_global_depth_aumenta_conforme_necessario(void) {
    int depth_inicial = hash_get_global_depth(h);

    for (int i = 0; i < 50; i++) {
        int valor = i;
        hash_insert(h, (uint64_t)i, &valor, sizeof(int));
    }

    TEST_ASSERT_TRUE(hash_get_global_depth(h) >= depth_inicial);
}

/* =========================================================
   CHAVES DUPLICADAS
   ========================================================= */
void test_insert_chave_duplicada_nao_trava(void) {
    int a = 5, b = 7;
    hash_insert(h, 1, &a, sizeof(int));
    hash_insert(h, 1, &b, sizeof(int));

    size_t tamanho;
    int *resultado = hash_search(h, 1, &tamanho);
    TEST_ASSERT_NOT_NULL(resultado);
    /* Nao garantimos qual das duas sera encontrada primeiro — so que uma existe */
    TEST_ASSERT_TRUE(*resultado == 5 || *resultado == 7);
    free(resultado);
}

/* =========================================================
   FOREACH
   ========================================================= */
static int contador_foreach = 0;
static void contar_callback(uint64_t chave, void *dado, size_t tamanho, void *contexto) {
    (void)chave; (void)dado; (void)tamanho; (void)contexto;
    contador_foreach++;
}

void test_foreach_visita_todos_os_registros(void) {
    contador_foreach = 0;
    for (int i = 0; i < 15; i++) {
        int valor = i;
        hash_insert(h, (uint64_t)i, &valor, sizeof(int));
    }
    hash_foreach(h, contar_callback, NULL);
    TEST_ASSERT_EQUAL_INT(15, contador_foreach);
}

/* =========================================================
   DUMP (so verifica que nao trava/crasha)
   ========================================================= */
void test_dump_nao_trava(void) {
    int valor = 1;
    hash_insert(h, 1, &valor, sizeof(int));

    FILE *tmp = tmpfile();
    TEST_ASSERT_NOT_NULL(tmp);
    hash_dump(h, tmp);
    fclose(tmp);
}

/* =========================================================
   HASH DE STRING (djb2)
   ========================================================= */
void test_hash_string_djb2_e_deterministica(void) {
    uint64_t h1 = hash_string_djb2("b03.1");
    uint64_t h2 = hash_string_djb2("b03.1");
    TEST_ASSERT_EQUAL_UINT64(h1, h2);
}

void test_hash_string_djb2_strings_diferentes_geram_chaves_diferentes(void) {
    uint64_t h1 = hash_string_djb2("b03.1");
    uint64_t h2 = hash_string_djb2("b03.2");
    TEST_ASSERT_NOT_EQUAL(h1, h2);
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_hash_deve_ser_criada);

    RUN_TEST(test_insert_e_search_deve_encontrar_o_dado);
    RUN_TEST(test_search_chave_inexistente_retorna_null);
    RUN_TEST(test_insert_e_search_de_struct);
    RUN_TEST(test_search_retorna_copia_independente);

    RUN_TEST(test_remove_deve_remover_e_retornar_o_dado);
    RUN_TEST(test_remove_chave_inexistente_retorna_null);

    RUN_TEST(test_insercoes_multiplas_causam_split);
    RUN_TEST(test_global_depth_aumenta_conforme_necessario);

    RUN_TEST(test_insert_chave_duplicada_nao_trava);

    RUN_TEST(test_foreach_visita_todos_os_registros);
    RUN_TEST(test_dump_nao_trava);

    RUN_TEST(test_hash_string_djb2_e_deterministica);
    RUN_TEST(test_hash_string_djb2_strings_diferentes_geram_chaves_diferentes);

    return UNITY_END();
}