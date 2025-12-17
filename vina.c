#include"options.h"


int main(int argc, char *argv[]) {
    // Verifica se a quantidade certa de argumentos foi passada
    if (argc < 3) {
        fprintf(stderr, "Uso: vinac <opção> <archive> [membro1 membro2 …]\n");
        return 1;
    }

    // Variaveis para receberem os argumentos
    const char *option = argv[1];
    const char *archive_name = argv[2];
    char **members = &argv[3];
    int num_members = argc - 3;

    // Verificadores de opção
    if (strcmp(option, "-x") == 0) {
        option_x(archive_name, num_members, members);
        return 0;
    }

    if (strcmp(option, "-c") == 0) {
        option_c(archive_name);
        return 0;
    }

    if (strcmp(option, "-ip") == 0 || strcmp(option, "-p") == 0) {
        option_ip_p(archive_name, num_members, members);
        return 0;
    }

    if (strcmp(option, "-ic") == 0 || strcmp(option, "-i") == 0) {
        option_ic_i(archive_name, num_members, members);
        return 0;
    }

    if (strcmp(option, "-m") == 0) {
        const char *to_move = argv[3];
        const char *target = argv[4];
        option_m(archive_name, to_move, target);
        return 0;
    }

    if (strcmp(option, "-r") == 0) {
        option_r(archive_name, num_members, members);
        return 0;
    }

    if(strcmp(option, "-z") == 0) {
        option_z(archive_name, num_members, members);
        return 0;
    }

    fprintf(stderr, "Opção desconhecida: %s\n", option);
    return 1;

}
