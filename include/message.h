#ifndef MESSAGE_H
#define MESSAGE_H

// Représente le message pour un nouveau client
typedef struct new_client_t {
    int codereq;
    int id;
    char pseudo[10];
} new_client_t;

// Représente un message envoyé par le client
typedef struct client_message_t {
    int codereq;
    int id;
    uint16_t numfil;
    uint16_t nb;
    uint8_t datalen;
    char *data;
} client_message_t;

// Représente le format UDP pour l'envoi de fichier
typedef struct udp_t {
    int codereq;
    int id;
    uint16_t numblock;
    char paquet[512];
} udp_t;

// Représente le format de la réponse du serveur
typedef struct server_message_t {
    int codereq;
    int id;
	uint16_t numfil;
	uint16_t nb;
} server_message_t;

// Représente le format pour s'inscrire un fil
typedef struct subscribe_t {
	server_message_t *server_message;
	uint16_t addrmult; // A passer en 128 bits
} subscribe_t;

// Représente le format d'un billet à renvoyer (server_message_t.nb fois ce message)
typedef struct post_t {
	uint16_t numfil;
	char origin[11];
	char pseudo[11];
	uint8_t datalen;
    char *data;
} post_t;

// Représente le format d'une notification
typedef struct notification_t {
	server_message_t *server_message;
	char pseudo[10];
	char data[20];
} notification_t;

// Crée une en-tête pour un message (avec mise au format big-endian)
uint16_t create_header(int codereq, int id);


// Client:

// New client:

// Crée un message pour l'inscription d'un nouveau client
new_client_t *create_new_client(char *pseudo);

// Envoie un `new client` sur le file descriptor donné
int send_new_client(int fd, new_client_t *new_client);

// Lit un `new client` (côté serveur)
new_client_t *read_to_new_client(char *data);

// Libère la mémoire allouée pour l'inscription du client
void delete_new_client(new_client_t *new_client);

// Message:

// Crée un message client
client_message_t *create_client_message(int codereq, int id, int numfil, int nb, int datalen, char *data);

// Envoie un message client sur le file descriptor donné
int send_client_message(int fd, client_message_t *client_message);

// Lit un message (côté serveur)
client_message_t *read_to_client_message(char *data);

// Libère la mémoire allouée pour un message client
void delete_client_message(client_message_t *client_message);


// UDP

// Crée une structure UDP
udp_t *create_udp(int id, uint16_t numblock, char *buf);

// Envoie une structure UDP sur le socket et l'adresse donnés
int send_udp(int sock, struct sockaddr_in6 addr, udp_t *udp);

// Convertis une chaîne de caractères en struct udp_t
udp_t *read_to_udp(char *buf);

// Libère la mémoire allouée par une structure UDP
void delete_udp(udp_t *udp);


// Server:

// Message:

// Crée un message serveur
server_message_t *create_server_message(int codereq, int id, int numfil, int nb);

// Envoie un message serveur sur le file descriptor donné
int send_server_message(int fd, server_message_t *server_message);

// Envoie un message d'erreur sur le file descriptor donné
void send_error_message(int fd);

// Lit un message (côté client)
server_message_t *read_server_message(int fd);

// Libère la mémoire allouée pour un message serveur
void delete_server_message(server_message_t *server_message);

// Lit les billets envoyés par le serveur
post_t *read_post(int fd);

// Envoyé les billets au client
int send_post(int fd, post_t *post);

// Libère la mémoire allouée pour un billet
void delete_post(post_t *post);

subscribe_t *create_subscribe_message(int codereq, int id, int numfil, int nb, int addrmult);
// codereq peut être inutile puisque toujours égal à 4

int send_subscribe_message(int fd, subscribe_t *subscribe_message);

void delete_subscribe_message(subscribe_t *subscribe_message);

notification_t *create_notification_message(int codereq, int id, int numfil, char *pseudo, char *data);
// codereq peut être inutile puisque toujours égal à 4

void delete_notification_message(notification_t *notification_message);

// Retire les dièse à la fin du pseudo
void remove_hash(char *pseudo);

#endif
