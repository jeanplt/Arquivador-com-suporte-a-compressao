#include"options.h"


/*************************************************************************
* FUNÇÕES AUXILIARES
*************************************************************************/

DirEntry* load_archive_meta(const char *archive_name, int *out_count) {
    // Abre o arquivo em modo leitura binária e em seguida lê o
    // offset para o início do diretório (verifica se são válidos)
    FILE *f = fopen(archive_name, "rb");
    if (!f) {
        *out_count = 0;
        return NULL;
    }

    // Offset com largura exata de 64 bits
    uint64_t dir_offset = 0;
    if (fread(&dir_offset, sizeof(dir_offset), 1, f) != 1 || dir_offset <= sizeof(dir_offset)) {
        fclose(f);
        *out_count = 0;
        return NULL;
    }

    // Busca o tamanho total do arquivo para saber quantos membros
    // possui e aloca um vetor para eles
    fseek(f, 0, SEEK_END);
    long file_end = ftell(f);
    int count = (file_end - dir_offset) / sizeof(DirEntry);
    DirEntry *entries = calloc(count, sizeof(DirEntry));

    // Faz a leitura dos membros a partir do início do diretório
    fseek(f, dir_offset, SEEK_SET);
    if(fread(entries, sizeof(DirEntry), count, f) != (size_t)count) {
        perror("Carregar membros de archive falhou");
        free(entries);
        fclose(f);
        *out_count = 0;
        return NULL;
    };
    fclose(f);

    *out_count = count;
    return entries;
}

void save_archive(const char *archive_name, MemberData *all, int total) {
    // Abre o arquivo em modo escrita binária e em seguida reserva
    // espaço para o ponteiro de início do diretório 
    FILE *f = fopen(archive_name, "wb+");
    long dir_ptr = sizeof(dir_ptr);
    fwrite(&dir_ptr, sizeof(dir_ptr), 1, f);

    long cur = dir_ptr;
    // For para gravar dados dos membros sequencialmente
    for (int i = 0; i < total; i++) {
        all[i].entry.order  = i;
        all[i].entry.offset = cur;
        fwrite(all[i].data, 1, all[i].entry.size_in_disk, f);
        cur += all[i].entry.size_in_disk;
    }
    // Outro For agora para gravar o diretório completo
    for (int i = 0; i < total; i++) {
        fwrite(&all[i].entry, sizeof(DirEntry), 1, f);
        free(all[i].data);
    }

    // Manda o ponteiro para o início do diretório
    fseek(f, 0, SEEK_SET);
    fwrite(&cur, sizeof(cur), 1, f);
    fclose(f);
}

MemberData* load_archive_data(const char *archive_name, DirEntry *entries, int count) {
    // Cria o vetor de MemberData e abre o Archive em modo leitura binária
    MemberData *all = calloc(count, sizeof(MemberData));
    FILE *f = fopen(archive_name, "rb");
    // For para copiar metadata, alocar buffer e posicionar no offset
    // dos dados
    for (int i = 0; i < count; i++) {
        all[i].entry = entries[i];
        size_t sz = entries[i].size_in_disk;
        all[i].data = malloc(sz);
        fseek(f, entries[i].offset, SEEK_SET);
        if(fread(all[i].data, 1, sz, f) != sz)
        {
            perror("Carregar o dado do membro falhou");
            free(all[i].data);
            all[i].data = NULL;
        }
    }
    fclose(f);
    return all;
}


/*************************************************************************
* FUNÇÕES PRINCIPAIS
*************************************************************************/

void option_ip_p(const char *archive_name, int num_members, char *members[])
{
    // Carrega o DirEntry do Archive e em seguida aloca espaço para um
    // vetor de MemberData com membros antigos e novos
    int old_count;
    DirEntry *old = load_archive_meta(archive_name, &old_count);
    MemberData *all = malloc((old_count + num_members) * sizeof(MemberData));
    int total = old_count;

    // Verifica se os membros já existem para carregar seus dados em Memória
    if (old_count) {
        MemberData *loaded = load_archive_data(archive_name, old, old_count);
        memcpy(all, loaded, old_count * sizeof(MemberData));
        free(loaded);
    }

    // For para processar cada arquivo novo da linha de comando
    for (int j = 0; j < num_members; j++) {
        const char *path = members[j];
        struct stat st;
        if (stat(path, &st) != 0) {
            perror(path);
            continue;
        }

        FILE *in = fopen(path, "rb");
        if (!in) {
            perror(path);
            continue;
        }

        unsigned char *buf = malloc(st.st_size);
        if (fread(buf, 1, st.st_size, in) != (size_t)st.st_size) {
            perror("Erro ao ler buffer");
            fclose(in);
            free(buf);
            continue;
        }
        fclose(in);

        // Preenchimento do DirEntry para o arquivo/membro
        DirEntry e = {0};
        strncpy(e.name, path, sizeof(e.name) - 1);
        e.uid           = st.st_uid;
        e.size_original = st.st_size;
        e.size_in_disk  = st.st_size;
        e.mtime         = st.st_mtime;

        // For e condicionais para verificar se substitui ou anexa
        // o membro
        int idx = -1;
        for (int i = 0; i < old_count; i++) {
            if (strcmp(all[i].entry.name, path) == 0) {
                idx = i;
                break;
            }
        }

        if (idx >= 0) {
            free(all[idx].data);
            all[idx].entry = e;
            all[idx].data  = buf;
        } else {
            all[total].entry = e;
            all[total].data  = buf;
            total++;
        }
    }

    // Salva o Archive atualizado e imprime uma mensagem para
    // dizer que concluiu o processo
    save_archive(archive_name, all, total);
    free(all);
    free(old);

    printf("Inseridos/substituídos membros em '%s'\n", archive_name);
}

void option_ic_i(const char *archive_name, int num_members, char *members[])
{
    int old_count;
    DirEntry *old = load_archive_meta(archive_name, &old_count);
    MemberData *all = malloc((old_count + num_members) * sizeof(MemberData));
    int total = old_count;

    if (old_count) {
        MemberData *loaded = load_archive_data(archive_name, old, old_count);
        memcpy(all, loaded, old_count * sizeof(MemberData));
        free(loaded);
    }

    for (int j = 0; j < num_members; j++) {
        const char *path = members[j];
        struct stat st;
        if (stat(path, &st) != 0) {
            perror(path);
            continue;
        }

        size_t insize = st.st_size;
        unsigned char *buf = malloc(insize + 1);
        if (!buf) {
            perror("Erro no malloc buf");
            continue;
        }

        FILE *in = fopen(path, "rb");
        if (!in || fread(buf, 1, insize, in) != insize) {
            perror("Erro ao ler buffer");
            if (in) fclose(in);
            free(buf);
            continue;
        }
        fclose(in);
        
        // Preenche com zero o ultimo indice de buf como segurança 
        // (Lz_Compress não acessar área inválida) e prepara um
        // buffer de saída para compressão (0.4% + 1)  
        buf[insize] = 0;
        size_t outcap = insize + (insize >> 8) + 1;
        unsigned char *outbuf = malloc(outcap);
        if (!outbuf) {
            perror("Erro no malloc outbuf");
            free(buf);
            continue;
        }

        // Variável para receber o tamanho da compressão e depois
        // verificar se valeu a pena ou não
        int cmp_sz = 0;
        if (insize > 0 && insize < LZ_MAX_OFFSET) {
            cmp_sz = LZ_Compress(buf, outbuf, (unsigned)insize);
        }

        // Verifica se deve usar comprimido por LZ ou o original
        unsigned char *bestbuf = buf;
        size_t bestsz = insize;
        if (cmp_sz > 0 && (size_t)cmp_sz < insize) {
            bestbuf = outbuf;
            bestsz  = cmp_sz;
        } else {
            free(outbuf);
            outbuf = NULL;
        }

        // Preenche o DirEntry com o melhor tamanho
        DirEntry e = {0};
        strncpy(e.name, path, sizeof(e.name) - 1);
        e.uid           = st.st_uid;
        e.size_original = st.st_size;
        e.size_in_disk  = bestsz;
        e.mtime         = st.st_mtime;

        int idx = -1;
        for (int i = 0; i < old_count; i++) {
            if (strcmp(all[i].entry.name, path) == 0) {
                idx = i;
                break;
            }
        }

        if (idx >= 0) {
            free(all[idx].data);
            all[idx].entry = e;
            all[idx].data  = bestbuf;
        } else {
            all[total].entry = e;
            all[total].data  = bestbuf;
            total++;
        }

        // Verifica os buffers para liberar não usados
        if (bestbuf == buf && outbuf) 
            free(outbuf);
        else if (bestbuf != buf) 
            free(buf);
    }

    save_archive(archive_name, all, total);
    free(all);
    free(old);

    printf("Inseridos/comprimidos membros em '%s'\n", archive_name);
}

void option_r(const char *archive_name, int num_members, char *members[])
{
    // Carrega os dados atuais dos membros em Memória
    int count;
    DirEntry *meta = load_archive_meta(archive_name, &count);
    if (count == 0) {
        printf("Nenhum membro em '%s'.\n", archive_name);
        return;
    }
    MemberData *all = load_archive_data(archive_name, meta, count);
    free(meta);

    // For para filtrar quais membros serão removidos do Archive
    int keep = 0;
    for (int i = 0; i < count; i++) {
        int to_remove = 0;
        for (int j = 0; j < num_members; j++) {
        if (strcmp(all[i].entry.name, members[j]) == 0) {
            to_remove = 1;
            break;
        } 
        }
        if (!to_remove) {
            all[keep++] = all[i];
        } else {
            free(all[i].data);
        }
    }

    save_archive(archive_name, all, keep);
    free(all);

    // Imprime membros passados para remoção para dizer que 
    // concluiu o processo 
    printf("Membros removidos (ou já não estavam) de '%s':\n", archive_name);
}

void option_c(const char *archive_path) {
    // Carrega todos os DirEntry de uma vez
    int num_membros = 0;
    DirEntry *entries = load_archive_meta(archive_path, &num_membros);
    if (num_membros == 0 || !entries) {
        printf("Nenhum membro no diretório de '%s'.\n", archive_path);
        free(entries);
        return;
    }

    // Abre Archive para obter o tamanho total do arquivo
    FILE *f = fopen(archive_path, "rb");
    if (!f) {
        perror("Erro ao abrir o archive");
        free(entries);
        return;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        perror("Erro no fseek");
        fclose(f);
        free(entries);
        return;
    }
    long file_end = ftell(f);
    if (file_end < 0) {
        perror("Erro no ftell");
        fclose(f);
        free(entries);
        return;
    }
    fclose(f);

    // Imprime o tamanho do Archive (debug) e em seguida a
    // quantidade de membros presentes nele
    printf("O tamanho do archive é: %ld bytes\n\n", file_end);
    printf("Número de membros: %d\n\n", num_membros);

    // For para imprimir cada propriedade especificada do
    // membro presente
    for (int i = 0; i < num_membros; i++) {
        DirEntry *e = &entries[i];

        char data_modif[20] = {0};
        // Formata a data de modificação com localtime e verifica 
        // se é válida
        struct tm *tm_info = localtime(&e->mtime);
        if (tm_info) {
            strftime(data_modif, sizeof(data_modif), "%d/%m/%Y %H:%M:%S", tm_info);
        } else {
            snprintf(data_modif, sizeof(data_modif), "Data inválida");
        }

        printf("Ordem: %d\n", e->order);
        printf("Nome: %s\n", e->name);
        printf("UID: %u\n", e->uid);
        printf("Tamanho original: %zu bytes\n", e->size_original);
        printf("Tamanho em disco: %zu bytes\n", e->size_in_disk);
        printf("Data de modificação: %s\n", data_modif);
        printf("Offset no arquivo: %ld\n", e->offset);
        printf("/**************************************/\n");
    }

    free(entries);
}

void option_x(const char *archive_name, int num_members, char *members[])
{
    // Carrega os dados atuais dos membros em Memória
    int count;
    DirEntry *entries = load_archive_meta(archive_name, &count);
    if (count == 0) {
    printf("Nenhum membro em '%s'.\n", archive_name);
    return;
    }

    FILE *f = fopen(archive_name, "rb");
    unsigned char *buffer = NULL;

    // For para marcar os membros que serão extraídos ou não
    for (int i = 0; i < count; i++) {
            int extrair = (num_members == 0) ? 1 : 0;
        for (int j = 0; j < num_members && !extrair; j++) {
            if (strcmp(entries[i].name, members[j]) == 0)
                extrair = 1;
            }
        if (!extrair) continue;

        // Lê os dados dos membros em buffer
        const DirEntry *e = &entries[i];
        fseek(f, e->offset, SEEK_SET);
        buffer = realloc(buffer, e->size_in_disk);
        size_t got = fread(buffer, 1, e->size_in_disk, f);
            if (got != e->size_in_disk) {
                fprintf(stderr, "Erro: só leu %zu de %zu bytes de %s\n", got, e->size_in_disk, e->name);
                continue;
            }

        // Abre arquivo em modo escrita binária para extrair os 
        // arquivos escolhidos originais ou então descompactados
        // se tamanho original > tamanho em disco
        FILE *out = fopen(e->name, "wb");
        if (e->size_in_disk < e->size_original) {
            unsigned char *decomp = malloc(e->size_original);
            LZ_Uncompress(buffer, decomp, (unsigned)e->size_in_disk);
            fwrite(decomp, 1, e->size_original, out);
            free(decomp);
        } else {
            fwrite(buffer, 1, e->size_original, out);
        }
        fclose(out);

        // Imprime o arquivo extraído e seu tamanho para dizer
        // que concluiu o processo
        printf("Extraído: %s (%zu bytes)\n", e->name, e->size_original);
    }

    free(buffer);
    free(entries);
    fclose(f);
}

void option_m(const char *archive_name, const char *src_name, const char *target_name)
{
    int count;
    DirEntry *meta = load_archive_meta(archive_name, &count);
    if (count == 0) {
        fprintf(stderr, "Nenhum membro em '%s'.\n", archive_name);
        return;
    }
    MemberData *all = load_archive_data(archive_name, meta, count);
    free(meta);

    // Variáveis e For para buscar o índice do membro a ser movido
    // e seu target
    int idx_src = -1; 
    int idx_target = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(all[i].entry.name, src_name) == 0)         
            idx_src = i;
        if (target_name && strcmp(all[i].entry.name, target_name) == 0)    
            idx_target = i;
    }
    
    // Verifica se o membro está no Archive
    if (idx_src < 0) {
        fprintf(stderr, "Membro '%s' não existe\n", src_name);
        
        // For para liberar o data buffer alocado antes de retornar
        for (int i = 0; i < count; i++) 
            free(all[i].data);
        
        free(all);
        return;
    }
    // Verifica se o target existe no Archive (ou no caso é NULL)
    if (target_name && idx_target < 0) {
        fprintf(stderr, "Target '%s' não existe\n", target_name);
        for (int i = 0; i < count; i++) free(all[i].data);
            free(all);
        return;
    }

    // Remove temporariamente o membro a ser movido e o memoriza
    MemberData moved = all[idx_src];
    
    // Desloca todos os elementos à direita de idx_src uma posição 
    // para a esquerda
    int qtd_to_move = count - idx_src - 1;
    memmove(&all[idx_src], &all[idx_src + 1], qtd_to_move * sizeof(MemberData));

    // Determina a nova posição de inserção, verificando se tem 
    // target especificado (inserir no início se não tiver) ou
    // caso contrário inserir depois do destino
    int ins;
    if (!target_name) {
        ins = 0;
    } else {
        // Verifica se target está após o membro a ser movido 
        // para poder descer sua posição após a remoção
        if (idx_target > idx_src) {
            idx_target--; 
        }
        ins = idx_target + 1;
    }

    // Abre espaço para inserção deslocando os elementos à direita 
    // de 'ins' e insere o membro movido na nova posição
    qtd_to_move = count - 1 - ins;
    memmove(&all[ins + 1], &all[ins], qtd_to_move * sizeof(MemberData));
    all[ins] = moved;

    save_archive(archive_name, all, count);

    // Libera a MemberData e imprime o membro mudado para dizer
    // que concluiu o processo
    free(all);
    printf("Movido %s em %s\n", src_name, archive_name);
}

void option_z(const char *archive_name, int num_members, char *members[]) 
{   
    int meta_count;
    DirEntry *meta = load_archive_meta(archive_name, &meta_count);
    
    // Verifica se o Archive tem algum membro, se não tiver 
    // encerra a operação
    if (meta_count == 0 || !meta) {
        free(meta);
        return;
    }

    int *idx = malloc(num_members * sizeof(int));
    for (int j = 0; j < num_members; j++) {
        idx[j] = -1;
        for (int i = 0; i < meta_count; i++) {
            if (strcmp(meta[i].name, members[j]) == 0) {
                idx[j] = i;
                break;
            }
        }
        // Verifica se o membro especificado existe em Archive, se
        // não existir encerra a operação
        if (idx[j] < 0) { 
            free(idx);
            free(meta);
            return;
        }
    }

    MemberData *all = load_archive_data(archive_name, meta, meta_count);
    free(meta);

    // Laço para buscar só membros relecionados
    MemberData *sel = malloc(num_members * sizeof(MemberData));
    for (int j = 0; j < num_members; j++) 
        sel[j] = all[idx[j]];
        
    // Laço para liberar membros não relacionados
    for (int i = 0; i < meta_count; i++) {
        int used = 0;
        for (int j = 0; j < num_members; j++)
            if (idx[j] == i) { 
                used = 1; 
                break; 
            }
        
        if (!used) 
            free(all[i].data);
    }
    free(all);
    free(idx);

    // Nomeia novo Archive e salva com _z
    size_t L = strlen(archive_name);
    char *new_name = malloc(L + 3 + 1 + 1); 
    char *dot = strrchr(archive_name, '.');
    if (dot && strcmp(dot, ".vc") == 0) {
        size_t base = dot - archive_name;
        memcpy(new_name, archive_name, base);
        strcpy(new_name + base, "_z.vc");
    } else {
        strcpy(new_name, archive_name);
        strcat(new_name, "_z.vc");
    }

    save_archive(new_name, sel, num_members);
    free(sel);

    printf("Arquivo derivado criado!\n");
    free(new_name);
}