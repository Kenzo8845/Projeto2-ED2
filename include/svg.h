#ifndef SVG_H
#define SVG_H

#include <stdio.h>
#include "quadra.h"
#include "hash.h"

/**
 * Este módulo lida com a escrita do arquivo svg e gera as saídas gráficas do sistema. 
 * Também é responsavel pelo ciclo de vida do arquivo svg. Quando abri-lo e fecha-lo.
 * É desacoplado do módulo Grafo: 
 * Ele só sabe desenhar formas geométricas rimitivas (retângulos, segmentos, textos, percursos animados). 
 * Quem decide oque desenhar (iterando o grafo, o resultado do Dijkstra, etc.) é o chamador (main.c / parser_qry.c).
 */

//******************************************** //
//           Ciclo de vida do documento 
//******************************************** //

/**
 * @brief Escreve a tag de abertura <svg> no arquivo.
 * @param arquivo Arquivo já aberto para escrita.
 */
void svg_iniciar(FILE *arquivo);

/**
 * @brief Escreve a tag de abertura <svg> já com o atributo viewBox calculado, de forma que todo o desenho caiba dentro da área visível.
 * @param arquivo Arquivo já aberto para escrita.
 * @param minx,miny Canto superior-esquerdo do conteúdo (sem margem).
 * @param maxx,maxy Canto inferior-direito do conteúdo (sem margem).
 * @param margem Folga extra adicionada em volta do conteúdo, nas 4 direções.
 */
void svg_iniciar_com_bbox(FILE *arquivo, double minx, double miny,
                          double maxx, double maxy, double margem);

/**
 * @brief Calcula o bounding box (menor retângulo que contém todas as quadras) percorrendo a hash de quadras.
 * @param tabela_quadras Hash contendo as quadras.
 * @param minx,miny,maxx,maxy Ponteiros de saída com os limites calculados. Se a hash estiver vazia, todos recebem 0.0.
 */
void svg_calcular_bbox_quadras(const Hash *tabela_quadras,
                               double *minx, double *miny,
                               double *maxx, double *maxy);

/**
 * @brief Escreve a tag de fechamento </svg> no arquivo.
 * @param arquivo Arquivo já aberto para escrita.
 */
void svg_fechar(FILE *arquivo);

/**
 * @brief Copia todo o conteúdo de 'src' (do início, via rewind) para o final
 * de 'dst'. Útil para montar um SVG final concatenando camadas que foram escritas em arquivos temporários separados.
 * @param dst Arquivo de destino.
 * @param src Arquivo de origem (será rebobinado).
 */
void svg_copiar(FILE *dst, FILE *src);

//******************************************** //
//           Quadras 
//******************************************** //

/**
 * @brief Desenha uma quadra (retângulo + rótulo com o CEP) no SVG.
 * @param arquivo Arquivo de saída.
 * @param q A quadra a ser desenhada.
 */
void svg_desenhar_quadra(FILE *arquivo, const Quadra q);

/**
 * @brief Desenha todas as quadras armazenadas na hash. Percorre a hash 
 * internamente (via hash_foreach) — o chamador não precisa conhecer a estrutura interna da hash.
 * @param arquivo Arquivo de saída.
 * @param tabela_quadras Hash contendo as quadras (chave = hash do CEP).
 */
void svg_desenhar_todas_quadras(FILE *arquivo, const Hash *tabela_quadras);

//******************************************** //
//           Primitivas genéricas (vias, MST, etc.) 
//******************************************** //

/**
 * @brief Desenha um segmento de reta simples (usado para vias, arestas ampliadas pelo exp, etc.).
 * @param arquivo Arquivo de saída.
 * @param x1,y1 Ponto inicial.
 * @param x2,y2 Ponto final.
 * @param cor Cor do traço (nome CSS ou #hex).
 * @param largura Espessura do traço (stroke-width).
 */
void svg_desenhar_segmento(FILE *arquivo, double x1, double y1, double x2, double y2,
                           const char *cor, double largura);

/**
 * @brief Desenha um retângulo semi-transparente (usado para bounding boxes de componentes conexos no comando regs).
 * @param arquivo Arquivo de saída.
 * @param minx,miny Canto superior-esquerdo do retângulo.
 * @param maxx,maxy Canto inferior-direito do retângulo.
 * @param cor Cor de preenchimento.
 * @param opacidade Opacidade do preenchimento (0.0 a 1.0; use 0.5 para 50%%).
 */
void svg_desenhar_retangulo_transparente(FILE *arquivo, double minx, double miny,
                                         double maxx, double maxy,
                                         const char *cor, double opacidade);

//******************************************** //
//           Comando @o? — marcador de endereço 
//******************************************** //

/**
 * @brief Desenha o marcador de um endereço geo-referenciado: uma linha vertical pontilhada vermelha da posição do endereço até o topo da página, com o número do registrador no topo.
 * @param arquivo Arquivo de saída.
 * @param x,y Coordenada do endereço.
 * @param id_registrador Texto do registrador (ex.: "R0") a ser mostrado no topo da linha.
 */
void svg_marcar_endereco(FILE *arquivo, double x, double y, const char *id_registrador);

//******************************************** //
//           Comando p? — percursos animados 
//******************************************** //

/**
 * @brief Desenha um percurso como um <path> poligonal e anima uma figura (círculo) percorrendo-o via <animateMotion>/<mpath>, conforme o padrão de animação exigido pelo enunciado.
 * @param arquivo Arquivo de saída.
 * @param xs Array com as coordenadas X dos pontos do percurso, em ordem (da origem ao destino).
 * @param ys Array com as coordenadas Y dos pontos do percurso, na mesma ordem.
 * @param n_pontos Número de pontos nos arrays xs/ys (deve ser >= 2).
 * @param cor Cor da linha do percurso e da figura animada.
 * @param path_id Identificador único (id SVG) para este path — necessário 
 * para o <mpath xlink:href="#id"> referenciar o caminho certo; use 
 * um id diferente para cada percurso desenhado no mesmo documento.
 * @param duracao_s Duração de uma volta completa da animação, em segundos.
 */
void svg_desenhar_percurso_animado(FILE *arquivo, const double *xs, const double *ys,
                                   int n_pontos, const char *cor,
                                   const char *path_id, double duracao_s);

/**
 * @brief Desenha uma "placa" circular com uma letra (usado para marcar os 
 * extremos Início/Fim de um percurso: letra 'I' e letra 'F').
 * @param arquivo Arquivo de saída.
 * @param x,y Coordenada onde centralizar a placa.
 * @param letra Letra a ser exibida ('I' ou 'F').
 * @param cor Cor de preenchimento da placa.
 */
void svg_marcar_placa(FILE *arquivo, double x, double y, char letra, const char *cor);

//******************************************** //
//           Wrappers usados por main.c / parserGeo.c / parserQry.c 
//******************************************** //

void svg_abrir(FILE *arquivo);

void svg_retangulo(FILE *arquivo, double x, double y, double w, double h,
                   const char *corp, const char *corb, double sw);

void svg_linha(FILE *arquivo, double x1, double y1, double x2, double y2,
               const char *cor, double largura, bool tracejado);

void svg_texto(FILE *arquivo, double x, double y, const char *texto,
               const char *cor, double tamanho_fonte);

void svg_retangulo_transparente(FILE *arquivo, double minx, double miny,
                                double maxx, double maxy, const char *cor);

void svg_caminho_animado(FILE *arquivo, const double *xs, const double *ys,
                         int n_pontos, const char *cor, double duracao_s);

void svg_placa(FILE *arquivo, double x, double y, char letra);

//******************************************** //
//           Grafo viário (camada base) 
//******************************************** //

/**
 * @brief Escreve o <defs> com o marcador de seta usado nas arestas do grafo viário. Deve ser chamado uma vez, logo após svg_iniciar 
 * (ou svg_iniciar_com_bbox) e antes de desenhar qualquer aresta.
 */
void svg_definir_marcador_seta(FILE *arquivo);

/**
 * @brief Desenha um vértice do grafo viário (pequeno círculo laranja).
 */
void svg_desenhar_vertice(FILE *arquivo, double x, double y);

/**
 * @brief Igual a svg_desenhar_vertice, mas também escreve um rótulo de texto
 * com o id do vértice logo acima do círculo (usado para a malha viária base, igual à Figura 5 do enunciado).
 * @param id String com o identificador do vértice (ex.: "v1", "v2.v3").
 */
void svg_desenhar_vertice_rotulado(FILE *arquivo, double x, double y, const char *id);

/**
 * @brief Desenha uma aresta do grafo viário: linha cinza com seta indicando
 * o sentido do tráfego (requer svg_definir_marcador_seta já ter sido chamado no documento).
 */
void svg_desenhar_aresta_viaria(FILE *arquivo, double x1, double y1, double x2, double y2);

#endif