#ifndef MST_H
#define MST_H

#include "grafo.h"

/**
 * Este módulo calcula a árvore geradora mínima (por comprimento) do grafo
 * viário via Kruskal, e usa o resultado para ampliar o tráfego dos trechos
 * "estruturais" que ainda estão com velocidade média insuficiente. Usado
 * pelo comando exp do arquivo .qry.
 */

/**
 * @brief Tipo de callback usado por mst_ampliar_vias.
 * @param origem Índice do vértice de origem da aresta ampliada.
 * @param destino Índice do vértice de destino da aresta ampliada.
 * @param cmp Comprimento da aresta ampliada, em metros.
 * @param vm_nova Nova velocidade média da aresta, após a ampliação de 50%.
 * @param contexto Ponteiro de contexto livre, repassado por mst_ampliar_vias.
 */
typedef void (*MstArestaCallback)(int origem, int destino, double cmp, double vm_nova, void *contexto);

/**
 * @brief Calcula a árvore geradora mínima do grafo (por comprimento, via
 * Kruskal) e, dentre as arestas selecionadas, aumenta em 50% a velocidade
 * média das que estiverem abaixo de vl. Para cada aresta ampliada, chama o
 * callback informando os novos dados.
 * @param g O grafo (as arestas selecionadas têm sua velocidade média alterada).
 * @param vl Velocidade mínima considerada "suficiente".
 * @param callback Função chamada para cada aresta ampliada.
 * @param contexto Ponteiro repassado a cada chamada do callback.
 * @return int Número de arestas ampliadas.
 */
int mst_ampliar_vias(Grafo *g, double vl, MstArestaCallback callback, void *contexto);

#endif