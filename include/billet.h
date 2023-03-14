#ifndef BILLET
#define BILLET

#include <unistd.h>

// Représente l'entête d'un billet
typedef struct entete_t {
    uint16_t entete;
} entete_t;

// Représente le billet pour un nouveau client
typedef struct new_client_t {
    entete_t *entete;
    char pseudo[10];
} new_client_t;

// Représente un billet envoyé par le client
typedef struct billet_t {
    entete_t *entete;
    uint16_t numfil;
    uint16_t nb;
    uint8_t datalen;
    char *data;
} billet_t;

// Représente le format UDP pour l'envoi de fichier
typedef struct udp_t {
    entete_t *entete;
    uint16_t numblock;
    char *paquet[512]; // A définir
} udp_t;

// Crée une entête
entete_t *create_entete(int codereq, int id);

// Libère la mémoire occupée par une entête
void delete_entete(entete_t *entete);

// Crée une requête pour l'inscription du client
new_client_t *create_new_client(char *pseudo);

// Libère la mémoire allouée pour l'inscription du client
void delete_new_client(new_client_t *new_client);

#endif
