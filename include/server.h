#ifndef SERVER_H
#define SERVER_H

typedef struct users_register_t {
	char **users;
	int new_id;
} users_register_t;

// Fonction pour threads
void *serve(void *arg);

// Structure des arguments des threads
typedef struct thread_args_t {
	int sock;
	struct sockaddr_in6 addr;
} thread_args_t;

// Gère la requête 1 pour inscription d'un nouvel utilisateur
int register_new_client(int sock, char *data);

typedef struct stack_post_t stack_post_t;
struct stack_post_t {
	post_t *post;
	stack_post_t *next;
};

typedef struct msg_thread_t {
	int nb_msg;
	char pseudo_init[10];
	stack_post_t *posts;
	char * multicast_ip;
	int	id[1024];
} msg_thread_t;

typedef struct msg_threads_register_t {
	msg_thread_t **msg_threads;
	int nb_fils;
} msg_threads_register_t;

// Gère la requête 5 pour ajout d'un fichier
int add_file(int sock, char *data);

// Gère la requête 6 pour téléchargement d'un fichier
int download_file(int sock, char *data, struct sockaddr_in6 addr);

// Alloue la mémoire nécessaire et initialise un registre vide
void create_register(void);

// Ajoute un pseudo et l'attribue à un ID
int add_user(char *pseudo);

// Retourne le pseudo correspondant à l'ID (à free), NULL si inexistant
char *get_user(int id);

// Libère la mémoire allouée pour le registre
void delete_register(void);

// Gère le signal SIGINT pour fermeture propre du serveur
void handler(int sig);

// Créer une pile de posts vide
stack_post_t *create_stack_post(void);

// Libère la mémoire allouée pour la pile
void delete_stack_post(stack_post_t *stack_post);

// Créer un fil avec le pseudo de l'initiateur du fil
msg_thread_t *create_msg_thread(char *pseudo);

// Ajoute un post au fil
int add_post(msg_thread_t *msg_thread, post_t *post);

// Libère la mémoire allouée pour le fil
void delete_msg_thread(msg_thread_t *msg_thread);

// Créer un registre de fils vide
void create_msg_threads_register(void);

// Ajoute un fil au registre retourne -1 si erreur 0 sinon
int add_msg_thread(msg_threads_register_t *msg_threads_register, msg_thread_t *msg_thread);

// Libère la mémoire allouée pour le registre
void delete_msg_threads_register(void);

// Recevoir et stocker un billet envoyé par un client
int receive_post(int sock, char *client_data);

// Envoyer les n derniers billets d'un fil à un client
int send_posts(int sock, char *client_data);

// Ajoute un fichier au fil donné
int add_file_to_thread(uint16_t numfil, file_t *file, int id);

// Crée le répertoire pour stocker les fichiers sur le serveur
int create_file_dir(void);

// Supprime le répertoire où sont stockés les fichiers
void delete_file_dir(void);

// Vérifie que la requête d'un client est valide
int is_client_valid(client_message_t *client_message);

int register_abonnement(int sock, char *data);

int	send_notification(int type, int fil, int id, char *pseudo);

#endif
