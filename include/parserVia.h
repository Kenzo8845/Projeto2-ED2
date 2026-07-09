#ifndef PARSER_VIA_H
#define PARSER_VIA_H

#include "grafo.h"

/** 
 * Este módulo lida com a leitura do arquivo via:
 * O arquivo via é responsável por ler e interpretar os dados de ruas e cruzamentos, 
 * instanciando os vértices e arestas.
 */

/**
 * @brief Lê o arquivo .via e popula a estrutura do grafo com seus vértices e arestas.
 * @param caminho_arquivo Caminho completo (ou relativo) para o arquivo .via.
 * @param g Ponteiro para o grafo (já inicializado) que será populado.
 * @return true se o arquivo foi processado com sucesso, false caso ocorra erro de abertura.
 */
bool parser_via_processar(const char* caminho_arquivo, Grafo* g);

#endif