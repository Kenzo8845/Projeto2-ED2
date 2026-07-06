#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>

/** 
 * Este módulo implementa uma Hash Extensível em memória.
 * Trabalha com pares (chave uint64_t, dado genérico). Ao inserir, a Hash 
 * copia os bytes do dado para dentro de si mesma — o chamador pode (e deve)
 * destruir sua cópia local logo após a inserção, sem medo de invalidar o
 * que foi armazenado.
 * O crescimento é feito via divisão (split) de buckets e, quando necessário,
 * duplicação do diretório — não há limite fixo de registros.
 */

typedef struct Hash Hash;

//******************************************** //
//           Constructor e Destructor
//******************************************** //

/**
 * @brief Cria uma Hash Extensível vazia.
 * @param capacidade_bucket Número máximo de registros por bucket antes de 
 * um split ser necessário. Deve ser >= 1.
 * @return Hash* Ponteiro para a hash recém-criada.
 */
Hash *hash_create(int capacidade_bucket);

/**
 * @brief Destroi a hash, liberando toda a memória interna (diretório, buckets e as cópias dos dados armazenados).
 * @param h Hash a ser destruída.
 */
void hash_destroy(Hash *h);

//******************************************** //
//           Operações principais 
//******************************************** //

/**
 * @brief Insere um dado na hash sob a chave informada. A hash copia 'tamanho' bytes a partir de 'dado' para armazenamento interno.
 * @param h Hash onde inserir.
 * @param chave Chave do registro.
 * @param dado Ponteiro para os dados a serem copiados e armazenados.
 * @param tamanho Tamanho em bytes de 'dado'.
 * @return true se a inserção teve sucesso.
 */
bool hash_insert(Hash *h, uint64_t chave, const void *dado, size_t tamanho);

/**
 * @brief Busca o registro associado à chave.
 * @param h Hash onde buscar.
 * @param chave Chave buscada.
 * @param tamanho Se não for NULL, recebe o tamanho em bytes do dado encontrado.
 * @return void* Cópia recém-alocada dos dados encontrados (o chamador deve dar free ou usar o destructor apropriado), ou NULL se não encontrado.
 */
void *hash_search(const Hash *h, uint64_t chave, size_t *tamanho);

/**
 * @brief Remove o registro associado à chave, se existir.
 * @param h Hash de onde remover.
 * @param chave Chave a ser removida.
 * @param tamanho Se não for NULL, recebe o tamanho em bytes do dado removido.
 * @return void* Cópia dos dados removidos (o chamador deve liberar), ou NULL se a chave não existia.
 */
void *hash_remove(Hash *h, uint64_t chave, size_t *tamanho);

//******************************************** //
//           Introspecção / Iteração 
//******************************************** //

/**
 * @brief Pega o número de buckets atualmente alocados pela hash.
 * @param h A hash.
 * @return int Número de buckets.
 */
int hash_get_num_buckets(const Hash *h);

/**
 * @brief Pega a profundidade global atual da hash.
 * @param h A hash.
 * @return int Profundidade global (log2 do tamanho do diretório).
 */
int hash_get_global_depth(const Hash *h);

/**
 * @brief Tipo de função de callback usado por hash_foreach.
 * @param chave Chave do registro visitado.
 * @param dado Ponteiro para os dados do registro (NÃO liberar; pertence à hash).
 * @param tamanho Tamanho em bytes do dado.
 * @param contexto Ponteiro de contexto livre, repassado por hash_foreach.
 */
typedef void (*HashCallback)(uint64_t chave, void *dado, size_t tamanho, void *contexto);

/**
 * @brief Percorre todos os registros válidos da hash, chamando o callback para cada um. 
 * Não copia os dados: o callback recebe um ponteiro direto para o armazenamento interno e não deve alterá-lo nem liberá-lo.
 * @param h A hash a ser percorrida.
 * @param callback Função chamada para cada registro.
 * @param contexto Ponteiro repassado a cada chamada do callback.
 */
void hash_foreach(const Hash *h, HashCallback callback, void *contexto);

/**
 * @brief Imprime um dump textual do estado interno da hash (diretório, 
 * buckets, profundidades locais e chaves armazenadas) em 'saida'. Útil para depuração.
 * @param h A hash.
 * @param saida Arquivo já aberto onde o dump será escrito.
 */
void hash_dump(const Hash *h, FILE *saida);

//******************************************** //
//           Utilitário de hashing 
//******************************************** //

/**
 * @brief Função hash de string (djb2), útil para transformar CEPs, ids de vértice, etc. em chaves uint64_t.
 * @param str String a ser transformada em chave.
 * @return uint64_t Chave resultante.
 */
uint64_t hash_string_djb2(const char *str);

#endif