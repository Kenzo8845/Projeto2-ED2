#include "../include/componentesConexos.h"
#include "../include/grafo.h"
#include "../Unity/unity.h"

static Grafo *g;
static int chamadas_callback;

static void callback_teste_comp(int comp_idx, double xmin, double ymin, double xmax, double ymax, void *ctx) {
    chamadas_callback++;
}

void setUp(void) {
    g = grafo_create(5);
    chamadas_callback = 0;
}

void tearDown(void) {
    grafo_destroy(g);
}

void test_componentes_conexos_grafo_vazio_retorna_zero(void) {
    int num_comp = componentes_conexos_calcular(g, 10.0, callback_teste_comp, NULL);
    TEST_ASSERT_EQUAL_INT(0, num_comp);
    TEST_ASSERT_EQUAL_INT(0, chamadas_callback);
}

void test_componentes_conexos_vertices_isolados(void) {
    grafo_add_vertice(g, "A", 0.0, 0.0);
    grafo_add_vertice(g, "B", 10.0, 10.0);
    
    int num_comp = componentes_conexos_calcular(g, 10.0, callback_teste_comp, NULL);
    TEST_ASSERT_EQUAL_INT(2, num_comp);
    TEST_ASSERT_EQUAL_INT(2, chamadas_callback);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_componentes_conexos_grafo_vazio_retorna_zero);
    RUN_TEST(test_componentes_conexos_vertices_isolados);
    return UNITY_END();
}