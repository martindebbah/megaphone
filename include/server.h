#ifndef SERVER_H
#define SERVER_H

typedef struct users_register_t {
	char **users;
	int new_id;
} users_register_t;

// Alloue la mémoire nécessaire et initialise un registre vide
void create_register(void);

// Ajoute un pseudo et l'attribue à un ID
void add_user(char *pseudo);

// Retourne le pseudo correspondant à l'ID (à free), NULL si inexistant
char *get_user(int id);

// Libère la mémoire allouée pour le registre
void delete_register(void);

#endif
