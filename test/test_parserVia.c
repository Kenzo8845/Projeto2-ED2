#include "../include/parserVia.h"
#include "../include/grafo.h"
#include "../Unity/unity.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static Grafo *g;
static char   caminho_temp[64];

static void escrever_via_temp(const char *conteudo) {
    snprintf(caminho_temp, sizeof(caminho_temp), "temp_test_%d.via", (int)getpid());
    FILE *f = fopen(caminho_temp, "w");
    TEST_ASSERT_NOT_NULL(f);
    fputs(conteudo, f);
    fclose(f);
}

void setUp(void) {
    g = grafo_create(8);
    caminho_temp[0] = '\0';
}

void tearDown(void) {
    grafo_destroy(g);
    if (caminho_temp[0] != '\0') {
        remove(caminho_temp);
    }
}

/* =========================================================
   COMANDO v — insercao de vertices
   ========================================================= */
void test_parser_via_insere_vertices(void) {
    escrever_via_temp(
        "v A 0.0 0.0\n"
        "v B 10.0 0.0\n"
    );

    bool ok = parser_via_processar(caminho_temp, g);

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(2, grafo_get_num_vertices(g));

    int idx_a = grafo_find_vertice(g, "A");
    int idx_b = grafo_find_vertice(g, "B");

    TEST_ASSERT_TRUE(idx_a >= 0);
    TEST_ASSERT_TRUE(idx_b >= 0);
    TEST_ASSERT_EQUAL_DOUBLE(0.0,  grafo_get_vertice_x(g, idx_a));
    TEST_ASSERT_EQUAL_DOUBLE(10.0, grafo_get_vertice_x(g, idx_b));
}

/* =========================================================
   COMANDO e — insercao de arestas
   Formato usado pelo parser: e origem destino ldir lesq cmp vm nome
   ========================================================= */
void test_parser_via_insere_aresta_entre_vertices_existentes(void) {
    escrever_via_temp(
        "v A 0.0 0.0\n"
        "v B 10.0 0.0\n"
        "e A B cep1 cep2 15.5 8.0 ruaTeste\n"
    );

    parser_via_processar(caminho_temp, g);

    int idx_a = grafo_find_vertice(g, "A");
    TEST_ASSERT_EQUAL_INT(1, grafo_get_num_arestas_saida(g, idx_a));
}

void test_parser_via_aresta_com_vertice_inexistente_nao_e_inserida(void) {
    escrever_via_temp(
        "v A 0.0 0.0\n"
        /* B nunca foi declarado com 'v' */
        "e A B cep1 cep2 15.5 8.0 ruaTeste\n"
    );

    parser_via_processar(caminho_temp, g);

    int idx_a = grafo_find_vertice(g, "A");
    TEST_ASSERT_TRUE(idx_a >= 0);
    TEST_ASSERT_EQUAL_INT(0, grafo_get_num_arestas_saida(g, idx_a));
}

/* =========================================================
   CAMPOS "-" (ausentes) DEVEM VIRAR STRING VAZIA NO GRAFO
   ========================================================= */
typedef struct {
    char ldir_capturado[64];
    char lesq_capturado[64];
    bool encontrou;
} CapturaAresta;

static void capturar_aresta_cb(int origem, int destino, const char *nome,
                               const char *ldir, const char *lesq,
                               double cmp, double *vm, void *contexto) {
    (void)origem; (void)destino; (void)nome; (void)cmp; (void)vm;
    CapturaAresta *cap = (CapturaAresta *)contexto;
    strncpy(cap->ldir_capturado, ldir ? ldir : "", 63);
    strncpy(cap->lesq_capturado, lesq ? lesq : "", 63);
    cap->encontrou = true;
}

void test_parser_via_campo_dash_vira_string_vazia(void) {
    escrever_via_temp(
        "v A 0.0 0.0\n"
        "v B 10.0 0.0\n"
        "e A B - - 15.5 8.0 ruaTeste\n"
    );

    parser_via_processar(caminho_temp, g);

    CapturaAresta cap = { "", "", false };
    grafo_foreach_aresta(g, capturar_aresta_cb, &cap);

    TEST_ASSERT_TRUE(cap.encontrou);
    TEST_ASSERT_EQUAL_STRING("", cap.ldir_capturado);
    TEST_ASSERT_EQUAL_STRING("", cap.lesq_capturado);
}

/* =========================================================
   MULTIPLOS VERTICES E ARESTAS
   ========================================================= */
void test_parser_via_grafo_completo(void) {
    escrever_via_temp(
        "v A 0.0 0.0\n"
        "v B 10.0 0.0\n"
        "v C 20.0 0.0\n"
        "e A B cep1 cep2 10.0 5.0 rua1\n"
        "e B C cep3 cep4 10.0 5.0 rua2\n"
    );

    parser_via_processar(caminho_temp, g);

    TEST_ASSERT_EQUAL_INT(3, grafo_get_num_vertices(g));

    int idx_a = grafo_find_vertice(g, "A");
    int idx_b = grafo_find_vertice(g, "B");
    int idx_c = grafo_find_vertice(g, "C");

    TEST_ASSERT_EQUAL_INT(1, grafo_get_num_arestas_saida(g, idx_a));
    TEST_ASSERT_EQUAL_INT(1, grafo_get_num_arestas_saida(g, idx_b));
    TEST_ASSERT_EQUAL_INT(0, grafo_get_num_arestas_saida(g, idx_c)); /* C so recebe */
}

/* =========================================================
   ARQUIVO INEXISTENTE
   ========================================================= */
void test_parser_via_arquivo_inexistente_retorna_false(void) {
    bool ok = parser_via_processar("arquivo_que_nao_existe_98765.via", g);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_EQUAL_INT(0, grafo_get_num_vertices(g));
}

/* =========================================================
   COMANDO DESCONHECIDO — ignorado sem travar
   ========================================================= */
void test_parser_via_comando_desconhecido_e_ignorado(void) {
    escrever_via_temp(
        "# isso e um comentario invalido\n"
        "v A 0.0 0.0\n"
    );

    bool ok = parser_via_processar(caminho_temp, g);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, grafo_get_num_vertices(g));
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parser_via_insere_vertices);
    RUN_TEST(test_parser_via_insere_aresta_entre_vertices_existentes);
    RUN_TEST(test_parser_via_aresta_com_vertice_inexistente_nao_e_inserida);
    RUN_TEST(test_parser_via_campo_dash_vira_string_vazia);
    RUN_TEST(test_parser_via_grafo_completo);
    RUN_TEST(test_parser_via_arquivo_inexistente_retorna_false);
    RUN_TEST(test_parser_via_comando_desconhecido_e_ignorado);

    return UNITY_END();
}