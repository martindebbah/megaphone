#ifndef FILE_H
#define FILE_H

#define NPAQUETS 65536

// Structure représentant les fichiers dans le serveur
typedef struct file_t {
    char *name;
    int name_len;
    udp_t *data[NPAQUETS];
} file_t;

// Crée un `file` de nom `name`
file_t *create_file(char *name, int name_len);

// Ajoute un paquet udp à `file`
int add_data(file_t *file, udp_t *data);

// Retourne 1 si `file` contient toutes les données, 0 sinon
int is_complete(file_t *file);

// Renvoie la taille du fichier sous forme de chaîne de caractères
char *str_of_size_file(file_t *file);

// Envoie un fichier sur un socket UDP, à l'adresse donnée. Retourne 0 en cas de succès, 1 sinon
int send_file(int sock, struct sockaddr_in6 addr, int file, int id);

// Lit un fichier depuis un socket UDP, retourne 0 en cas de succès, 1 sinon
int recv_file(int sock, file_t *file);

// Ecrit un fichier sur le disque dur
int write_file(char *path, file_t *file);

// Libère la mémoire allouée pour `file`
void delete_file(file_t *file);

#endif
