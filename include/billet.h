#ifndef BILLET
#define BILLET

#include <unistd.h>

// Représente le billet pour un nouveau client
typedef struct new_client_t {
    uint16_t entete;
    char pseudo[10];
} new_client_t;

// Représente un billet envoyé par le client
typedef struct billet_t {
    uint16_t entete;
    uint16_t numfil;
    uint16_t nb;
    uint8_t datalen;
    char *data;
} billet_t;

// Représente le format UDP pour l'envoi de fichier
typedef struct udp_t {
    uint16_t entete;
    uint16_t numblock;
    char *paquet[512]; // A définir
} udp_t;

// Crée une entête
uint16_t create_entete(int codereq, int id);


// New client:

// Crée une requête pour l'inscription du client
new_client_t *create_new_client(char *pseudo);

// Envoie un `new client` sur le file descriptor donné
int send_new_client(int fd, new_client_t *new_client);

// Lire un `new client` (server)
void read_new_client(int fd, new_client_t *new_client);

// Libère la mémoire allouée pour l'inscription du client
void delete_new_client(new_client_t *new_client);


// Billet:

// Crée une requête pour l'envoi d'un billet
billet_t *create_billet(int codereq, int id, int numfil, int nb, int datalen, char *data);

// Envoie un billet sur le file descriptor donné
int send_billet(int fd, billet_t *billet);

// Lit un billet (server)
void read_billet(int fd, billet_t *billet);

// Libère la mémoire allouée pour un billet
void delete_billet(billet_t *billet);

#endif
