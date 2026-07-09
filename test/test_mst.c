#include "../include/mst.h"
#include "../include/grafo.h"
#include "../Unity/unity.h"

static Grafo *g;
static int arestas_ampliadas;

static void callback_teste_mst(int origem, int destino, double cmp, double vm_nova, void *ctx) {
    arestas_ampliadas++;
}

void setUp(void) {
    g = grafo_create(5);
    arestas_ampliadas = 0;
}

void tearDown(void) {
    grafo_destroy(g);
}

void test_mst_grafo_vazio_nao_faz_nada(void) {
    int result = mst_ampliar_vias(g, 50.0, callback_teste_mst, NULL);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(0, arestas_ampliadas);
}

void test_mst_amplia_via_abaixo_da_velocidade(void) {
    grafo_add_vertice(g, "A", 0.0, 0.0);
    grafo_add_vertice(g, "B", 10.0, 0.0);
    
    grafo_add_aresta(g, "A", "B", "Rua1", NULL, NULL, 10.0, 30.0);
    
    mst_ampliar_vias(g, 50.0, callback_teste_mst, NULL);
    
    TEST_ASSERT_GREATER_THAN_INT(0, arestas_ampliadas); 
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mst_grafo_vazio_nao_faz_nada);
    RUN_TEST(test_mst_amplia_via_abaixo_da_velocidade);
    return UNITY_END();
}