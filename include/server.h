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

#endif
