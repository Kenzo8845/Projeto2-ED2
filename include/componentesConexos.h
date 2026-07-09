#ifndef COMPONENTES_CONEXOS_H
#define COMPONENTES_CONEXOS_H

#include "grafo.h"

/**
 * Este módulo calcula os componentes conexos do grafo viário, considerando
 * como "suficiente" (e portanto elegível para conectar dois vértices) toda
 * aresta cuja velocidade média seja >= a um limiar informado. Usado pelo
 * comando regs do arquivo .qry para identificar as "ilhas" viárias da cidade.
 */

/**
 * @brief Tipo de callback usado por componentes_conexos_calcular.
 * @param comp_idx Índice do componente conexo encontrado.
 * @param xmin Coordenada x mínima do bounding box do componente.
 * @param ymin Coordenada y mínima do bounding box do componente.
 * @param xmax Coordenada x máxima do bounding box do componente.
 * @param ymax Coordenada y máxima do bounding box do componente.
 * @param contexto Ponteiro de contexto livre, repassado por componentes_conexos_calcular.
 */
typedef void (*ComponenteCallback)(int comp_idx, double xmin, double ymin, double xmax, double ymax, void *contexto);

/**
 * @brief Calcula os componentes conexos do grafo considerando apenas as
 * arestas com velocidade média >= vl (o grafo é tratado como não-direcionado
 * para fins de conectividade). Para cada componente encontrado, chama o
 * callback informando o bounding box dos vértices que o compõem.
 * @param g O grafo (não é modificado).
 * @param vl Velocidade mínima considerada "suficiente".
 * @param callback Função chamada para cada componente encontrado.
 * @param contexto Ponteiro repassado a cada chamada do callback.
 * @return int Número de componentes conexos encontrados.
 */
int componentes_conexos_calcular(Grafo *g, double vl, ComponenteCallback callback, void *contexto);

#endif