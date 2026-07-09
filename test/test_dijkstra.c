#include "../include/dijkstra.h"
#include "../include/grafo.h"
#include "../Unity/unity.h"
#include <float.h>
#include <stdlib.h>

static Grafo *g;
static int    *antecessores;
static double *distancias;

static void montar_grafo_padrao(void) {
    g = grafo_create(8);

    grafo_add_vertice(g, "A", 0.0, 0.0);
    grafo_add_vertice(g, "B", 10.0, 0.0);
    grafo_add_vertice(g, "C", 0.0, 10.0);
    grafo_add_vertice(g, "D", 20.0, 0.0);
    grafo_add_vertice(g, "E", 50.0, 50.0);

    grafo_add_aresta(g, "A", "B", "ruaAB", NULL, NULL, 10.0, 5.0);
    grafo_add_aresta(g, "B", "D", "ruaBD", NULL, NULL, 10.0, 5.0);
    grafo_add_aresta(g, "A", "D", "ruaAD", NULL, NULL, 30.0, 30.0);
    grafo_add_aresta(g, "A", "C", "ruaAC", NULL, NULL, 5.0, 5.0);
}

void setUp(void) {
    montar_grafo_padrao();
    int n = grafo_get_num_vertices(g);
    antecessores = malloc(sizeof(int) * (size_t)n);
    distancias = malloc(sizeof(double) * (size_t)n);
}

void tearDown(void) {
    free(antecessores);
    free(distancias);
    grafo_destroy(g);
}

/* =========================================================
   CASOS BASICOS
   ========================================================= */
void test_dijkstra_encontra_caminho_ate_o_destino(void) {
    int idx_a = grafo_find_vertice(g, "A");
    int idx_d = grafo_find_vertice(g, "D");

    bool achou = dijkstra_buscar(g, idx_a, idx_d, false, antecessores, distancias);

    TEST_ASSERT_TRUE(achou);
    TEST_ASSERT_TRUE(distancias[idx_d] < DBL_MAX);
}

void test_dijkstra_por_distancia_prefere_caminho_mais_curto(void) {
    /* A->B->D tem distancia 20, menor que A->D direto (30) */
    int idx_a = grafo_find_vertice(g, "A");
    int idx_d = grafo_find_vertice(g, "D");

    dijkstra_buscar(g, idx_a, idx_d, false, antecessores, distancias);

    TEST_ASSERT_EQUAL_DOUBLE(20.0, distancias[idx_d]);
}

void test_dijkstra_por_tempo_pode_preferir_caminho_diferente(void) {
    int idx_a = grafo_find_vertice(g, "A");
    int idx_d = grafo_find_vertice(g, "D");

    dijkstra_buscar(g, idx_a, idx_d, true, antecessores, distancias);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, distancias[idx_d]);
    TEST_ASSERT_EQUAL_INT(idx_a, antecessores[idx_d]);
}

/* =========================================================
   INALCANCAVEL
   ========================================================= */
void test_dijkstra_no_isolado_retorna_false(void) {
    int idx_a = grafo_find_vertice(g, "A");
    int idx_e = grafo_find_vertice(g, "E"); /* sem nenhuma aresta */

    bool achou = dijkstra_buscar(g, idx_a, idx_e, false, antecessores, distancias);

    TEST_ASSERT_FALSE(achou);
    TEST_ASSERT_EQUAL_DOUBLE(DBL_MAX, distancias[idx_e]);
    TEST_ASSERT_EQUAL_INT(-1, antecessores[idx_e]);
}

/* =========================================================
   ORIGEM == DESTINO
   ========================================================= */
void test_dijkstra_origem_igual_destino(void) {
    int idx_a = grafo_find_vertice(g, "A");

    bool achou = dijkstra_buscar(g, idx_a, idx_a, false, antecessores, distancias);

    TEST_ASSERT_TRUE(achou);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, distancias[idx_a]);
}

/* =========================================================
   INDICES INVALIDOS
   ========================================================= */
void test_dijkstra_indice_origem_invalido_retorna_false(void) {
    int num_v = grafo_get_num_vertices(g);
    bool achou = dijkstra_buscar(g, -1, 0, false, antecessores, distancias);
    TEST_ASSERT_FALSE(achou);

    achou = dijkstra_buscar(g, num_v + 10, 0, false, antecessores, distancias);
    TEST_ASSERT_FALSE(achou);
}

void test_dijkstra_indice_destino_invalido_retorna_false(void) {
    int idx_a = grafo_find_vertice(g, "A");
    int num_v = grafo_get_num_vertices(g);

    bool achou = dijkstra_buscar(g, idx_a, num_v + 10, false, antecessores, distancias);
    TEST_ASSERT_FALSE(achou);
}

/* =========================================================
   RASTREAMENTO DO CAMINHO (antecessores)
   ========================================================= */
void test_dijkstra_antecessores_permitem_reconstruir_caminho(void) {
    int idx_a = grafo_find_vertice(g, "A");
    int idx_d = grafo_find_vertice(g, "D");
    int idx_b = grafo_find_vertice(g, "B");

    dijkstra_buscar(g, idx_a, idx_d, false, antecessores, distancias);

    /* Caminho mais curto (por distancia) e A->B->D */
    TEST_ASSERT_EQUAL_INT(idx_b, antecessores[idx_d]);
    TEST_ASSERT_EQUAL_INT(idx_a, antecessores[idx_b]);
    TEST_ASSERT_EQUAL_INT(-1, antecessores[idx_a]); /* origem nao tem antecessor */
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_dijkstra_encontra_caminho_ate_o_destino);
    RUN_TEST(test_dijkstra_por_distancia_prefere_caminho_mais_curto);
    RUN_TEST(test_dijkstra_por_tempo_pode_preferir_caminho_diferente);

    RUN_TEST(test_dijkstra_no_isolado_retorna_false);
    RUN_TEST(test_dijkstra_origem_igual_destino);

    RUN_TEST(test_dijkstra_indice_origem_invalido_retorna_false);
    RUN_TEST(test_dijkstra_indice_destino_invalido_retorna_false);

    RUN_TEST(test_dijkstra_antecessores_permitem_reconstruir_caminho);

    return UNITY_END();
}