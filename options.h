/*************************************************************************
* Definições básicas para o Header e outras bibliotecas
*************************************************************************/

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "lz.h"

#define TEMP_NAME "archive.tmp" // nome base para arquivo temporário nas funções 
#define LZ_MAX_OFFSET 100000 // valor máximo suportado da compressão


/*************************************************************************
* STRUCTS
*************************************************************************/

/*************************************************************************
* DirEntry - Struct responsável pelas seguintes propriedades dos membros
* no Archive:
*  - Nome                 - Nome do membro, com tamanho de até 1024B 
*  - UID                  - UID do usuário do programa
*  - Tamanho Original     - Tamanho do membro sem compressão 
*  - Tamanho em Disco     - Tamanho do membro com compressão LZ77
*  - Data de Modificação  - Data do sistema em que o membro foi alterado
*  - Ordem no Archive     - Ordem do membro presente no Archive
*  - Localização          - Inicio do membro em Archive (offset)
*************************************************************************/
typedef struct {
    char name[1024];
    uid_t uid;
    size_t size_original;
    size_t size_in_disk;
    time_t mtime;
    int order;
    long offset;
} DirEntry;

/*************************************************************************
* MemberData - Struct responsável por toda a estrutura do membro no 
* Archive com:
*  - DirEntry  - Struct com as propriedades do membro
*  - data      - Conteúdo do membro
*************************************************************************/
typedef struct {
    DirEntry entry;
    unsigned char *data;
} MemberData;




/*************************************************************************
* PROTÓTIPOS DAS FUNÇÕES AUXILIARES
*************************************************************************/

/*************************************************************************
* load_archive_meta() - Carrega os metadados do Archive especificado.
*  - archive_name     - Caminho do Archive passado por comando
*  - out_cont         - Número a ser carregado do total de elementos em 
*                       Archive
* Retorna um vetor alocado de DirEntry com *out_cont elementos. Se o 
* Archive não existir, retorna *out_cont = 0 e o DirEntry será NULL.
*************************************************************************/
DirEntry* load_archive_meta(const char *archive_name, int *out_count);


/*************************************************************************
* save_archive() - Reescreve o Archive com novos dados dos membros.
*  - archive_name  - Caminho do Archive passado por comando
*  - all           - Dados de cada um dos novos e velhos membros
*  - total         - Quantidade de membros presentes em all
*************************************************************************/
void save_archive(const char *archive_name, MemberData *all, int total);


/*************************************************************************
* load_archive_data() - Carrega os conteúdos dos membros de forma 
*                       vetorizada do Archive especificado.
*  - archive_name     - Caminho do Archive passado por comando
*  - entries          - Propriedades dos membros em Archive
*  - count            - Quantidade de membros presentes
* Retorna um vetor MemberData com .entry copiado e .data alocado e 
* preenchido com conteúdo em disco.
*************************************************************************/
MemberData* load_archive_data(const char *archive_name, DirEntry *entries, int count);




/*************************************************************************
* PROTÓTIPOS DAS FUNÇÕES PRINCIPAIS
*************************************************************************/

/*************************************************************************
* option_ip_p() - Insere/acrescenta um ou mais membros sem compressão ao
*                 archive. Caso o membro já exista no archive, ele é 
*                 substituído. Novos membros são inseridos respeitando a 
*                 ordem da linha de comando, ao final do archive.
*  - archive_name     - Caminho do Archive passado por comando
*  - num_members      - Quantidade de arquivos existentes por comando
*  - members          - Vetor de caminhos para arquivos dados por comando
*************************************************************************/
void option_ip_p(const char *archive_name, int num_members, char *members[]);


/*************************************************************************
* option_ic_i() - Insere/acrescenta um ou mais membros com compressão ao
*                 archive. Caso o membro já exista no archive, ele é 
*                 substituído. Novos membros são inseridos respeitando a 
*                 ordem da linha de comando, ao final do archive.
*  - archive_name     - Caminho do Archive passado por comando
*  - num_members      - Quantidade de arquivos existentes por comando
*  - members          - Vetor de caminhos para arquivos dados por comando
*************************************************************************/
void option_ic_i(const char *archive_name, int num_members, char *members[]);


/*************************************************************************
* option_r() - Remove os membros indicados de Archive.
*  - archive_name     - Caminho do Archive passado por comando
*  - num_members      - Quantidade de arquivos existentes por comando
*  - members          - Vetor de caminhos para arquivos dados por comando
*************************************************************************/
void option_r(const char *archive_name, int num_members, char *members[]);


/*************************************************************************
* option_c() - Lista o conteúdo de archive em ordem, incluindo as 
*              propriedades de cada membro (nome, UID, tamanho original, 
*              tamanho em disco e data de modificação) e sua ordem no 
*              arquivo.
*  - archive_path     - Caminho do Archive passado por comando
*************************************************************************/
void option_c(const char *archive_path);


/*************************************************************************
* option_x() - Extrai os membros indicados de archive. Se os membros não 
*              forem indicados, todos devem ser extraídos. A extração 
*              consiste em ler o membro de archive e criar um arquivo 
*              correspondente, com conteúdo idêntico, em disco;
*  - archive_name     - Caminho do Archive passado por comando
*  - num_members      - Quantidade de arquivos existentes por comando
*  - members          - Vetor de caminhos para arquivos dados por comando
*************************************************************************/
void option_x(const char *archive_name, int num_members, char *members[]);


/*************************************************************************
* option_m() - Move o membro indicado na linha de comando para 
*              imediatamente depois do membro target existente em 
*              archive. A movimentação deve ocorrer na seção de dados do 
*              archive; para mover para o início, o target deve ser NULL.
*  - archive_name     - Caminho do Archive passado por comando
*  - src_name         - Nome do membro que será movido em Archive
*  - taget_name       - Nome do membro target em Archive (caso tenha um)
*************************************************************************/
void option_m(const char *archive_name, const char *src_name, const char *target_name);

/*************************************************************************
* option_z() - Cria, a partir de um arquivo .vc existente, um segundo 
*              arquivo .vc que contenha apenas um subconjunto selecionado 
*              dos membros do primeiro arquivo.       
*  - archive_name     - Caminho do Archive passado por comando
*  - num_members      - Quantidade de arquivos existentes por comando
*  - members          - Vetor de caminhos para arquivos dados por comando
*************************************************************************/
void option_z(const char *archive_name, int num_members, char *members[]);

#endif 

