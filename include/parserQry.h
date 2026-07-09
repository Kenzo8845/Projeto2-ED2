#ifndef PARSER_QRY_H
#define PARSER_QRY_H

#include "hash.h"
#include "grafo.h"
#include <stdio.h>

/** 
 * Este módulo lida com a leitura do arquivo qry
 * o arquivo qry é responsavel por passar os comandos que modificam os grafos e por fim,
 * o txt e o svg final.
 * Comandos:
 * @o? - reg cep face num 
 *      Armazena a posição geográfica do endereço cep/face/num no registrador reg.
 *      SVG: linha vertical pontilhada vermelha mostrando a posição do endereço e colocar o
 *      número do registrador na outra extremidade do linha (topo da página)
 *      TXT: reportar a coordenada relativa ao endereço
 * mvm - v x y w h 
 *      Atualiza a velocidade média das arestas dentro da região (x,y,w,h) para v
 * regs - vl 
 *      Considera como insuficiente os trechos com velocidade média inferior a vl. Calcula os componentes conexos.
 *      TXT: reportar o número de componente conexos.
 *      SVG: Calcular os bounding boxes de cada componente conexo e desenhar os respectivos retângulos com cores diferentes e transparência de 50%.
 * exp - vl 
 *      Calcula a árvore geradora mínima, seleciona apenas as arestas com velocidade média inferior a vl, aumenta em 50% a velocidade média das arestas selecionadas
 *      SVG: pintar as arestas selecionadas com linha grossa e vermelha. 
 * p? - reg1 reg2 cc cr 
 *      Determina o melhor trajeto entre a origem (reg1) e o destino (reg2). Desenhar os percursos (mais curto, mais rápido) com as cores cc e cr, respectivamente.
 *      TXT: descrição textual do percurso. Caso destino inacessível, reportar.
 *      SVG: mostrar os percursos pintados e animados.Marcar os extremos do percurso com “placas” com as letras I e F.
 */


/** 
 * @brief Processa o ficheiro de consultas (.qry), executando os comandos solicitados.
 * @param caminho_qry Caminho para o ficheiro .qry a ser lido.
 * @param quadras Tabela hash contendo as quadras da cidade.
 * @param vias Grafo contendo o sistema viário (vértices e arestas).
 * @param txt_saida Ficheiro .txt aberto para escrita do relatório textual.
 * @param svg_saida Ficheiro .svg aberto para escrita dos desenhos e trajetos.
 */
void parser_qry_processar(const char* caminho_qry, Hash* quadras, Grafo* vias, FILE* txt_saida, FILE* svg_saida);

#endif