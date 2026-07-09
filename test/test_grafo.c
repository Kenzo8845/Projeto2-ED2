#include "../include/grafo.h"
#include "../Unity/unity.h"

static Grafo *g;

void setUp(void) {
    g = grafo_create(5);
}

void tearDown(void) {
    grafo_destroy(g);
}

void test_grafo_deve_ser_criado_vazio(void) {
    TEST_ASSERT_NOT_NULL(g);
    TEST_ASSERT_EQUAL_INT(0, grafo_get_num_vertices(g));
}

void test_grafo_adicionar_vertice(void) {
    int v1 = grafo_add_vertice(g, "V1", 10.0, 20.0);
    TEST_ASSERT_EQUAL_INT(0, v1);
    TEST_ASSERT_EQUAL_INT(1, grafo_get_num_vertices(g));
    TEST_ASSERT_EQUAL_DOUBLE(10.0, grafo_get_vertice_x(g, v1));
    TEST_ASSERT_EQUAL_DOUBLE(20.0, grafo_get_vertice_y(g, v1));
}

void test_grafo_nao_duplica_vertice_existente(void) {
    int v1 = grafo_add_vertice(g, "V1", 10.0, 20.0);
    int v2 = grafo_add_vertice(g, "V1", 50.0, 50.0); // Mesmo ID
    TEST_ASSERT_EQUAL_INT(v1, v2);
    TEST_ASSERT_EQUAL_INT(1, grafo_get_num_vertices(g)); // Continua 1
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_grafo_deve_ser_criado_vazio);
    RUN_TEST(test_grafo_adicionar_vertice);
    RUN_TEST(test_grafo_nao_duplica_vertice_existente);
    return UNITY_END();
}