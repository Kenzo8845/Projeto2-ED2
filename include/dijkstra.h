#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <stdbool.h>
#include "grafo.h"

/**
 * Este módulo implementa o algoritmo de Dijkstra para caminhos mínimos sobre
 * o Grafo viário, usando uma fila de prioridade (min-heap) binária.
 * Suporta dois critérios de custo por aresta:
 *   - distância (cmp), para o percurso mais curto;
 *   - tempo (cmp / vm), para o percurso mais rápido.
 */

/**
 * @brief Calcula o caminho de menor custo de idx_origem até idx_destino.
 * @param g Grafo onde buscar.
 * @param idx_origem Índice do vértice de origem.
 * @param idx_destino Índice do vértice de destino.
 * @param por_tempo Se true, o custo de cada aresta é cmp/vm (tempo); se 
 * false, o custo é cmp (distância).
 * @param out_antecessores Array (tamanho >= grafo_get_num_vertices(g)) que 
 * recebe, para cada vértice, o índice de seu antecessor no caminho 
 * mínimo a partir da origem (-1 se não alcançado ou é a própria 
 * origem).
 * @param out_distancias Array (mesmo tamanho) que recebe o custo acumulado 
 * mínimo até cada vértice (DBL_MAX se inalcançável).
 * @return true se idx_destino foi alcançado a partir de idx_origem.
 */
bool dijkstra_buscar(const Grafo *g, int idx_origem, int idx_destino, bool por_tempo, int *out_antecessores, double *out_distancias);

#endif