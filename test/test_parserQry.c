#include "../include/parserQry.h"
#include "../include/hash.h"
#include "../include/grafo.h"
#include "../Unity/unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static Hash *quadras;
static Grafo *vias;

void setUp(void) {
    quadras = hash_create(4);
    vias = grafo_create(5);
}

void tearDown(void) {
    hash_destroy(quadras);
    grafo_destroy(vias);
}

void test_parser_qry_arquivo_inexistente_ignorado(void) {
    FILE *txt = tmpfile();
    FILE *svg = tmpfile();
    
    // Tentar processar um arquivo que não existe não deve dar crash
    parser_qry_processar("arquivo_inexistente_9999.qry", quadras, vias, txt, svg);
    
    TEST_ASSERT_TRUE(1); // Passou se não deu SegFault
    
    fclose(txt);
    fclose(svg);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parser_qry_arquivo_inexistente_ignorado);
    return UNITY_END();
}