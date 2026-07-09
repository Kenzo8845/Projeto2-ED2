#ifndef GRAFO_H
#define GRAFO_H

#include <stdbool.h>

/**
 * Este módulo implementa um grafo direcionado via listas de adjacência.
 * Os vértices representam extremos de segmentos de rua (id, x, y).
 * As arestas representam o segmento de rua em si, e guardam nome da rua,
 * cep das quadras à direita (ldir) e à esquerda (lesq), comprimento (cmp)
 * e velocidade média (vm). Vértices e arestas são identificados apenas
 * pelo índice inteiro atribuído no momento da inserção.
 */

typedef struct Grafo Grafo;

//******************************************** //
//           Constructor e Destructor
//******************************************** //

/**
 * @brief Cria um grafo vazio.
 * @param capacidade_inicial Capacidade inicial do array de vértices (>= 1).
 * @return Grafo* Ponteiro para o grafo recém-criado.
 */
Grafo *grafo_create(int capacidade_inicial);

/**
 * @brief Destroi o grafo, liberando toda a memória interna (vértices e arestas).
 * @param g Grafo a ser destruído.
 */
void grafo_destroy(Grafo *g);

//******************************************** //
//           Inserção
//******************************************** //

/**
 * @brief Insere um vértice no grafo, ou retorna o índice se o id já existir.
 * @param g Grafo onde inserir.
 * @param id Identificador (string) do vértice.
 * @param x Coordenada x do vértice.
 * @param y Coordenada y do vértice.
 * @return int Índice do vértice.
 */
int grafo_add_vertice(Grafo *g, const char *id, double x, double y);

/**
 * @brief Insere uma aresta direcionada (id_origem -> id_destino). Os vértices
 * envolvidos devem já existir no grafo.
 * @param g Grafo onde inserir.
 * @param id_origem Id do vértice de origem.
 * @param id_destino Id do vértice de destino.
 * @param nome Nome da rua a qual a aresta pertence.
 * @param ldir Cep da quadra à direita do segmento (ou "-"/NULL se ausente).
 * @param lesq Cep da quadra à esquerda do segmento (ou "-"/NULL se ausente).
 * @param cmp Comprimento do segmento, em metros.
 * @param vm Velocidade média no segmento, em m/s.
 * @return true se a aresta foi inserida, false se origem ou destino não existem.
 */
bool grafo_add_aresta(Grafo *g, const char *id_origem, const char *id_destino, const char *nome, const char *ldir, const char *lesq, double cmp, double vm);

//******************************************** //
//           Consulta
//******************************************** //

/**
 * @brief Busca o índice de um vértice a partir do seu id.
 * @param g O grafo.
 * @param id Id buscado.
 * @return int Índice do vértice, ou -1 se não encontrado.
 */
int grafo_find_vertice(const Grafo *g, const char *id);

/**
 * @brief Pega o número de vértices do grafo.
 * @param g O grafo.
 * @return int Número de vértices.
 */
int grafo_get_num_vertices(const Grafo *g);

/**
 * @brief Pega o id de um vértice a partir do seu índice.
 * @param g O grafo.
 * @param idx Índice do vértice.
 * @return const char* Id do vértice.
 */
const char *grafo_get_vertice_id(const Grafo *g, int idx);

/**
 * @brief Pega a coordenada x de um vértice.
 * @param g O grafo.
 * @param idx Índice do vértice.
 * @return double Coordenada x.
 */
double grafo_get_vertice_x(const Grafo *g, int idx);

/**
 * @brief Pega a coordenada y de um vértice.
 * @param g O grafo.
 * @param idx Índice do vértice.
 * @return double Coordenada y.
 */
double grafo_get_vertice_y(const Grafo *g, int idx);

/**
 * @brief Pega o número de arestas de saída de um vértice.
 * @param g O grafo.
 * @param idx Índice do vértice.
 * @return int Número de arestas de saída.
 */
int grafo_get_num_arestas_saida(const Grafo *g, int idx);

//******************************************** //
//           Iteração
//******************************************** //

/**
 * @brief Tipo de callback usado por grafo_foreach_vizinho.
 * @param destino Índice do vértice de destino da aresta.
 * @param cmp Comprimento da aresta, em metros.
 * @param vm Velocidade média da aresta, em m/s.
 * @param contexto Ponteiro de contexto livre, repassado por grafo_foreach_vizinho.
 */
typedef void (*GrafoVizinhoCallback)(int destino, double cmp, double vm, void *contexto);

/**
 * @brief Percorre todas as arestas de saída de um vértice, chamando o callback
 * para cada uma. Útil para Dijkstra e buscas em geral.
 * @param g O grafo.
 * @param origem Índice do vértice de origem.
 * @param callback Função chamada para cada aresta de saída.
 * @param contexto Ponteiro repassado a cada chamada do callback.
 */
void grafo_foreach_vizinho(const Grafo *g, int origem, GrafoVizinhoCallback callback, void *contexto);

/**
 * @brief Tipo de callback usado por grafo_foreach_aresta.
 * @param origem Índice do vértice de origem.
 * @param destino Índice do vértice de destino.
 * @param nome Nome da rua da aresta.
 * @param ldir Cep da quadra à direita ("" se ausente).
 * @param lesq Cep da quadra à esquerda ("" se ausente).
 * @param cmp Comprimento da aresta, em metros.
 * @param vm Ponteiro para a velocidade média da aresta (pode ser alterado
 * diretamente pelo callback, por exemplo para os comandos mvm e exp).
 * @param contexto Ponteiro de contexto livre, repassado por grafo_foreach_aresta.
 */
typedef void (*GrafoArestaCallback)(int origem, int destino, const char *nome, const char *ldir, const char *lesq, double cmp, double *vm, void *contexto);

/**
 * @brief Percorre todas as arestas do grafo, chamando o callback para cada uma.
 * @param g O grafo.
 * @param callback Função chamada para cada aresta.
 * @param contexto Ponteiro repassado a cada chamada do callback.
 */
void grafo_foreach_aresta(Grafo *g, GrafoArestaCallback callback, void *contexto);

//******************************************** //
//           Busca espacial
//******************************************** //

/**
 * @brief Procura o vértice do grafo geometricamente mais próximo de um ponto
 * (x, y). Usado para associar um endereço (obtido via @o?) ao cruzamento ou
 * vértice de meio-de-quadra mais próximo, para fins de cálculo de percurso.
 * @param g O grafo.
 * @param x Coordenada x do ponto de referência.
 * @param y Coordenada y do ponto de referência.
 * @return int Índice do vértice mais próximo, ou -1 se o grafo estiver vazio.
 */
int grafo_procurar_vertice_proximo(const Grafo *g, double x, double y);

//******************************************** //
//           Atualização de tráfego
//******************************************** //

/**
 * @brief Atualiza a velocidade média de toda aresta cujos dois vértices
 * (origem e destino) estejam dentro da região retangular (x, y, w, h).
 * Usado pelo comando mvm. w e h negativos são tratados normalizando a região.
 * @param g O grafo.
 * @param v Nova velocidade média a ser atribuída.
 * @param x Coordenada x da âncora da região.
 * @param y Coordenada y da âncora da região.
 * @param w Largura da região.
 * @param h Altura da região.
 * @return int Número de arestas atualizadas.
 */
int grafo_atualizar_velocidade_regiao(Grafo *g, double v, double x, double y, double w, double h);

#endif