#ifndef FILE_H
#define FILE_H

#define NPAQUETS 65536

// Structure représentant les fichiers dans le serveur
typedef struct file_t {
    char *name;
    udp_t *data[NPAQUETS];
} file_t;

// Crée un `file` de nom `name`
file_t *create_file(char *name, int name_len);

// Ajoute un paquet udp à `file`
int add_data(file_t *file, udp_t *data);

// Retourne 1 si `file` contient toutes les données, 0 sinon
int is_complete(file_t *file);

// Libère la mémoire allouée pour `file`
void delete_file(file_t *file);

#endif
