#include "../include/quadra.h"
#include "../Unity/unity.h"
#include <string.h>

static Quadra q;

void setUp(void) {
    q = quadra_constructor("cep15", 100.0, 200.0, 50.0, 30.0);
}

void tearDown(void) {
    quadra_destructor(q);
}

/* =========================================================
   CRIACAO
   ========================================================= */
void test_quadra_deve_ser_criada(void) {
    TEST_ASSERT_NOT_NULL(q);
}

void test_quadra_valores_iniciais_corretos(void) {
    TEST_ASSERT_EQUAL_STRING("cep15", quadra_getCep(q));
    TEST_ASSERT_EQUAL_DOUBLE(100.0, quadra_getX(q));
    TEST_ASSERT_EQUAL_DOUBLE(200.0, quadra_getY(q));
    TEST_ASSERT_EQUAL_DOUBLE(50.0, quadra_getW(q));
    TEST_ASSERT_EQUAL_DOUBLE(30.0, quadra_getH(q));
}

void test_quadra_cores_iniciam_vazias(void) {
    /* calloc zera a struct: sem cq definido, corp/corb devem ser string vazia */
    TEST_ASSERT_EQUAL_STRING("", quadra_getCorp(q));
    TEST_ASSERT_EQUAL_STRING("", quadra_getCorb(q));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, quadra_getSw(q));
}

/* =========================================================
   GETTERS / SETTERS
   ========================================================= */
void test_quadra_setX_altera_valor(void) {
    quadra_setX(q, 555.5);
    TEST_ASSERT_EQUAL_DOUBLE(555.5, quadra_getX(q));
}

void test_quadra_setY_altera_valor(void) {
    quadra_setY(q, 321.0);
    TEST_ASSERT_EQUAL_DOUBLE(321.0, quadra_getY(q));
}

void test_quadra_setW_e_setH_alteram_valores(void) {
    quadra_setW(q, 10.0);
    quadra_setH(q, 20.0);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, quadra_getW(q));
    TEST_ASSERT_EQUAL_DOUBLE(20.0, quadra_getH(q));
}

void test_quadra_setCep_altera_valor(void) {
    quadra_setCep(q, "cep99");
    TEST_ASSERT_EQUAL_STRING("cep99", quadra_getCep(q));
}

void test_quadra_setCorp_e_setCorb_alteram_cores(void) {
    quadra_setCorp(q, "Moccasin");
    quadra_setCorb(q, "dodgerblue");
    TEST_ASSERT_EQUAL_STRING("Moccasin", quadra_getCorp(q));
    TEST_ASSERT_EQUAL_STRING("dodgerblue", quadra_getCorb(q));
}

void test_quadra_setSw_altera_espessura(void) {
    quadra_setSw(q, 2.5);
    TEST_ASSERT_EQUAL_DOUBLE(2.5, quadra_getSw(q));
}

/* =========================================================
   TAMANHO DA STRUCT
   ========================================================= */
void test_quadra_getSize_bate_com_uso_no_hash(void) {
    size_t s1 = quadra_getSize();
    size_t s2 = quadra_getSize();
    TEST_ASSERT_TRUE(s1 > 0);
    TEST_ASSERT_EQUAL_INT(s1, s2);
}

/* =========================================================
   ROBUSTEZ — strings no limite do buffer interno
   ========================================================= */
void test_quadra_cep_no_limite_do_buffer(void) {
    quadra_setCep(q, "123456789012345"); 
    TEST_ASSERT_EQUAL_STRING("123456789012345", quadra_getCep(q));
}

void test_quadra_cor_longa_e_truncada_com_seguranca(void) {
    char cor_longa[101];
    memset(cor_longa, 'A', 100);
    cor_longa[100] = '\0';

    quadra_setCorp(q, cor_longa);

    const char *resultado = quadra_getCorp(q);
    TEST_ASSERT_NOT_NULL(resultado);
    TEST_ASSERT_TRUE(strlen(resultado) <= 63);
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_quadra_deve_ser_criada);
    RUN_TEST(test_quadra_valores_iniciais_corretos);
    RUN_TEST(test_quadra_cores_iniciam_vazias);

    RUN_TEST(test_quadra_setX_altera_valor);
    RUN_TEST(test_quadra_setY_altera_valor);
    RUN_TEST(test_quadra_setW_e_setH_alteram_valores);
    RUN_TEST(test_quadra_setCep_altera_valor);
    RUN_TEST(test_quadra_setCorp_e_setCorb_alteram_cores);
    RUN_TEST(test_quadra_setSw_altera_espessura);

    RUN_TEST(test_quadra_getSize_bate_com_uso_no_hash);

    RUN_TEST(test_quadra_cep_no_limite_do_buffer);
    RUN_TEST(test_quadra_cor_longa_e_truncada_com_seguranca);

    return UNITY_END();
}