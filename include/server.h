#ifndef SERVER_H
#define SERVER_H

typedef struct users_register_t {
	char **users;
	int new_id;
} users_register_t;

// Fonction pour threads
void *serve(void *arg);

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
} msg_thread_t;

typedef struct msg_threads_register_t {
	msg_thread_t **msg_threads;
	int nb_fils;
} msg_threads_register_t;

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
void add_post(msg_thread_t *msg_thread, post_t *post);

// Libère la mémoire allouée pour le fil
void delete_msg_thread(msg_thread_t **msg_thread);

// Créer un registre de fils vide
void create_msg_threads_register(void);

// Ajoute un fil au registre
void add_msg_thread(msg_threads_register_t *msg_threads_register, msg_thread_t *msg_thread);

// Libère la mémoire allouée pour le registre
void delete_msg_threads_register(void);

// Recevoir et stocker un billet envoyé par un client
int receive_post(int sock, char *client_data);

// Envoyer les n derniers billets d'un fil à un client
int send_posts(int sock, char *client_data);

#endif
