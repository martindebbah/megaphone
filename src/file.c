#include "../include/megaphone.h"

file_t *create_file(char *name, int name_len) {
    file_t *file = calloc(1, sizeof(file_t));
    if (!file) {
        perror("Erreur calloc file");
        goto error;
    }

    file -> name = calloc(name_len + 1, 1);
    if (!file -> name) {
        perror("Erreur calloc file name");
        goto error;
    }

    memmove(file -> name, name, name_len);
    return file;

    error:
        if (file)
            delete_file(file);
        return NULL;
}

int add_data(file_t *file, udp_t *data) {
    if (file -> data[data -> numblock])
        return 1;
    file -> data[data -> numblock] = data;
    return 0;
}

int is_complete(file_t *file) {
    int complete = 0;
    for (int i = 0; i < NPAQUETS; i++) {
        if (!file -> data[i])
            break;
        if (strnlen(file -> data[i] -> paquet, 512) < 512)
            complete = 1;
    }
    return complete;
}

void delete_file(file_t *file) {
    if (!file)
        return;
    if (file -> name)
        free(file -> name);
    for (int i = 0; i < NPAQUETS; i++)
        if (file -> data[i])
            delete_udp(file -> data[i]);
    free(file);
}

udp_t *create_udp(char *data) {
    return NULL;
}

void delete_udp(udp_t *udp) {
    if (udp)
        free(udp);
}
