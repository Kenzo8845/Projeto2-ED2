#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parserVia.h"

bool parser_via_processar(const char* caminho_arquivo, Grafo* g) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de vias: %s\n", caminho_arquivo);
        return false;
    }

    char comando[10];
    
    char id_v[128];
    double x, y;

    char id_origem[128], id_destino[128], nome_rua[256], ldir[128], lesq[128];
    double cmp, vm;

    while (fscanf(arquivo, "%s", comando) != EOF) {
        
        // Comando 'v': Adiciona Vértice
        if (strcmp(comando, "v") == 0) {
            if (fscanf(arquivo, "%s %lf %lf", id_v, &x, &y) == 3) {
                grafo_add_vertice(g, id_v, x, y);
            }
        }
        
        // Comando 'e': Adiciona Aresta (Segmento de Rua)
        else if (strcmp(comando, "e") == 0) {
            if (fscanf(arquivo, "%s %s %s %s %lf %lf %s", 
                       id_origem, id_destino, ldir, lesq, &cmp, &vm, nome_rua) == 7) {
                
                const char *p_ldir = (strcmp(ldir, "-") == 0) ? NULL : ldir;
                const char *p_lesq = (strcmp(lesq, "-") == 0) ? NULL : lesq;

                bool inserido = grafo_add_aresta(g, id_origem, id_destino, nome_rua, p_ldir, p_lesq, cmp, vm);
                
                if (!inserido) {
                    printf("Aviso: Falha ao inserir aresta de %s para %s (vertices nao encontrados).\n", id_origem, id_destino);
                }
            }
        }
        
        else {
            char c;
            while ((c = fgetc(arquivo)) != '\n' && c != EOF);
        }
    }

    fclose(arquivo);
    return true;
}