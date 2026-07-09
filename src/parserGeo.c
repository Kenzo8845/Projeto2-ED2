#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/parserGeo.h"
#include "../include/quadra.h"
#include "../include/svg.h"

void parser_geo_processar(const char *caminho_arquivo, Hash *quadras, FILE *svg_geo) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo .geo: %s\n", caminho_arquivo);
        return;
    }

    char cor_preenchimento_atual[64] = "gray";
    char cor_borda_atual[64] = "black";
    double espessura_atual = 1.0;

    char comando[32];
    char cep[64];
    double x, y, w, h;
    char token_sw[32];

    while (fscanf(arquivo, "%s", comando) != EOF) {

        if (strcmp(comando, "cq") == 0) {
            if (fscanf(arquivo, "%s %s %s", token_sw, cor_preenchimento_atual, cor_borda_atual) != 3) {
                printf("Erro ao ler parametros do comando cq\n");
            } else {
                espessura_atual = atof(token_sw);
            }
        }

        else if (strcmp(comando, "q") == 0) {
            if (fscanf(arquivo, "%s %lf %lf %lf %lf", cep, &x, &y, &w, &h) == 5) {
                Quadra nova_q = quadra_constructor(cep, x, y, w, h);
                assert(nova_q != NULL);

                quadra_setCorp(nova_q, cor_preenchimento_atual);
                quadra_setCorb(nova_q, cor_borda_atual);
                quadra_setSw(nova_q, espessura_atual);

                if (svg_geo != NULL) {
                    svg_retangulo(svg_geo, x, y, w, h, cor_preenchimento_atual, cor_borda_atual, espessura_atual);
                }

                uint64_t chave_cep = hash_string_djb2(cep);
                hash_insert(quadras, chave_cep, nova_q, quadra_getSize());
                quadra_destructor(nova_q);
            }
        }

        else {
            char c;
            while ((c = fgetc(arquivo)) != '\n' && c != EOF);
        }
    }

    fclose(arquivo);
}