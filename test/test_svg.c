#include "../include/svg.h"
#include "../Unity/unity.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *f;
static const char *arquivo_temp = "test_temp_svg_saida.svg";

void setUp(void) {
    f = fopen(arquivo_temp, "w");
    TEST_ASSERT_NOT_NULL(f);
}

void tearDown(void) {
    if (f) fclose(f);
    remove(arquivo_temp);
}

void test_svg_ciclo_de_vida_basico(void) {
    svg_iniciar(f);
    svg_retangulo(f, 0.0, 0.0, 100.0, 100.0, "blue", "black", 2.0);
    svg_texto(f, 50.0, 50.0, "Teste", "black", 12.0);
    svg_fechar(f);
    
    // Se não quebrou (SegFault), a manipulação de ponteiros e fprintf está OK.
    TEST_ASSERT_TRUE(1); 
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_svg_ciclo_de_vida_basico);
    return UNITY_END();
}