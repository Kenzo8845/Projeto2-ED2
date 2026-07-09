#include "../include/parserGeo.h"
#include "../include/quadra.h"
#include "../include/hash.h"
#include "../Unity/unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static Hash *quadras;
static char  caminho_temp[64];

static void escrever_geo_temp(const char *conteudo) {
    snprintf(caminho_temp, sizeof(caminho_temp), "temp_test_%d.geo", (int)getpid());
    FILE *f = fopen(caminho_temp, "w");
    TEST_ASSERT_NOT_NULL(f);
    fputs(conteudo, f);
    fclose(f);
}

void setUp(void) {
    quadras = hash_create(4);
    caminho_temp[0] = '\0';
}

void tearDown(void) {
    hash_destroy(quadras);
    if (caminho_temp[0] != '\0') {
        remove(caminho_temp);
    }
}

/* =========================================================
   COMANDO q — insercao basica
   ========================================================= */
void test_parser_geo_insere_uma_quadra(void) {
    escrever_geo_temp("q cep15 100.0 200.0 50.0 30.0\n");

    parser_geo_processar(caminho_temp, quadras, NULL);

    uint64_t chave = hash_string_djb2("cep15");
    size_t tamanho;
    Quadra q = (Quadra)hash_search(quadras, chave, &tamanho);

    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("cep15", quadra_getCep(q));
    TEST_ASSERT_EQUAL_DOUBLE(100.0, quadra_getX(q));
    TEST_ASSERT_EQUAL_DOUBLE(200.0, quadra_getY(q));
    TEST_ASSERT_EQUAL_DOUBLE(50.0, quadra_getW(q));
    TEST_ASSERT_EQUAL_DOUBLE(30.0, quadra_getH(q));

    quadra_destructor(q);
}

/* =========================================================
   COMANDO cq — cor atual aplicada as quadras seguintes
   ========================================================= */
void test_parser_geo_cq_aplica_cor_as_quadras_seguintes(void) {
    escrever_geo_temp(
        "cq 2.0 Moccasin dodgerblue\n"
        "q cep20 0.0 0.0 10.0 10.0\n"
    );

    parser_geo_processar(caminho_temp, quadras, NULL);

    uint64_t chave = hash_string_djb2("cep20");
    size_t tamanho;
    Quadra q = (Quadra)hash_search(quadras, chave, &tamanho);

    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("Moccasin", quadra_getCorp(q));
    TEST_ASSERT_EQUAL_STRING("dodgerblue", quadra_getCorb(q));
    TEST_ASSERT_EQUAL_DOUBLE(2.0, quadra_getSw(q));

    quadra_destructor(q);
}

void test_parser_geo_quadra_antes_do_cq_usa_cor_padrao(void) {
    escrever_geo_temp(
        "q cep_antes 0.0 0.0 10.0 10.0\n"
        "cq 2.0 red blue\n"
    );

    parser_geo_processar(caminho_temp, quadras, NULL);

    uint64_t chave = hash_string_djb2("cep_antes");
    size_t tamanho;
    Quadra q = (Quadra)hash_search(quadras, chave, &tamanho);

    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("gray", quadra_getCorp(q));
    TEST_ASSERT_EQUAL_STRING("black", quadra_getCorb(q));

    quadra_destructor(q);
}

/* =========================================================
   MULTIPLAS QUADRAS
   ========================================================= */
void test_parser_geo_multiplas_quadras_sao_todas_inseridas(void) {
    escrever_geo_temp(
        "q cepA 0.0  0.0  10.0 10.0\n"
        "q cepB 20.0 0.0  10.0 10.0\n"
        "q cepC 40.0 0.0  10.0 10.0\n"
    );

    parser_geo_processar(caminho_temp, quadras, NULL);

    const char *ceps[] = { "cepA", "cepB", "cepC" };
    for (int i = 0; i < 3; i++) {
        uint64_t chave = hash_string_djb2(ceps[i]);
        size_t tamanho;
        Quadra q = (Quadra)hash_search(quadras, chave, &tamanho);
        TEST_ASSERT_NOT_NULL(q);
        TEST_ASSERT_EQUAL_STRING(ceps[i], quadra_getCep(q));
        quadra_destructor(q);
    }
}

/* =========================================================
   ARQUIVO INEXISTENTE — nao deve travar o programa
   ========================================================= */
void test_parser_geo_arquivo_inexistente_nao_trava(void) {
    parser_geo_processar("arquivo_que_nao_existe_12345.geo", quadras, NULL);
    uint64_t chave = hash_string_djb2("qualquer");
    size_t tamanho;
    void *resultado = hash_search(quadras, chave, &tamanho);
    TEST_ASSERT_NULL(resultado);
}

/* =========================================================
   COMANDO DESCONHECIDO — deve ser ignorado sem travar
   ========================================================= */
void test_parser_geo_comando_desconhecido_e_ignorado(void) {
    escrever_geo_temp(
        "xyz alguma coisa estranha aqui\n"
        "q cep_valido 5.0 5.0 15.0 15.0\n"
    );

    parser_geo_processar(caminho_temp, quadras, NULL);

    uint64_t chave = hash_string_djb2("cep_valido");
    size_t tamanho;
    Quadra q = (Quadra)hash_search(quadras, chave, &tamanho);
    TEST_ASSERT_NOT_NULL(q);
    quadra_destructor(q);
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parser_geo_insere_uma_quadra);
    RUN_TEST(test_parser_geo_cq_aplica_cor_as_quadras_seguintes);
    RUN_TEST(test_parser_geo_quadra_antes_do_cq_usa_cor_padrao);
    RUN_TEST(test_parser_geo_multiplas_quadras_sao_todas_inseridas);
    RUN_TEST(test_parser_geo_arquivo_inexistente_nao_trava);
    RUN_TEST(test_parser_geo_comando_desconhecido_e_ignorado);

    return UNITY_END();
}