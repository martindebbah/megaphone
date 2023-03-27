#ifndef MESSAGE_H
#define MESSAGE_H

#include <unistd.h>

// Représente le message pour un nouveau client
typedef struct new_client_t {
    uint16_t header;
    char pseudo[10];
} new_client_t;

// Représente un message envoyé par le client
typedef struct client_message_t {
    uint16_t header;
    uint16_t numfil;
    uint16_t nb;
    uint8_t datalen;
    char *data;
} client_message_t;

// Représente le format UDP pour l'envoi de fichier
typedef struct udp_t {
    uint16_t header;
    uint16_t numblock;
    char *paquet[512]; // A définir
} udp_t;

// Représente le format de la réponse du serveur
typedef struct server_message_t {
    uint16_t header;
	uint16_t numfil;
	uint16_t nb;
} server_message_t;

// Représente le format pour s'inscrire un fil
typedef struct subscribe_t {
	server_message_t *server_message;
	uint8_t addrmult[16];
} subscribe_t;

// Représente le format d'un billet à renvoyer (server_message_t.nb fois ce message)
typedef struct post_t {
	uint16_t numfil;
	char origin[10];
	char pseudo[10];
	uint8_t datalen;
    char *data;
} post_t;

// Représente le format d'une notification
typedef struct notification_t {
	server_message_t *server_message;
	char pseudo[10];
	char data[20];
} notification_t;

// Crée une en-tête pour un message
uint16_t create_header(int codereq, int id);


// Client:

// New client:

// Crée un message pour l'inscription d'un nouveau client
new_client_t *create_new_client(char *pseudo);

// Envoie un `new client` sur le file descriptor donné
int send_new_client(int fd, new_client_t *new_client);

// Lire un `new client` (server)
void read_new_client(int fd, new_client_t *new_client);

// Libère la mémoire allouée pour l'inscription du client
void delete_new_client(new_client_t *new_client);

// Message:

// Crée un message client
client_message_t *create_client_message(int codereq, int id, int numfil, int nb, int datalen, char *data);

// Envoie un message sur le file descriptor donné
int send_client_message(int fd, client_message_t *client_message);

// Lit un message
void read_client_message(int fd);

// Libère la mémoire allouée pour un message client
void delete_client_message(client_message_t *client_message);


// Server:

// Message:

// Crée un message serveur
server_message_t *create_server_message(int codereq, int id, int numfil, int nb);

int send_server_message(int fd, server_message_t *server_message);



// Libère la mémoire allouée pour un message serveur
void delete_server_message(server_message_t *server_message);

subscribe_t *create_subscribe_message(int codereq, int id, int numfil, int nb);

#endif
