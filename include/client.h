#ifndef CLIENT_H
#define CLIENT_H

// Crée une connexion avec le serveur et renvoie la socket associée (-1 en cas d'erreur)
int connect_to_server(void);

// Crée une connexion IPv4 avec le serveur et renvoie la socket associée (-1 en cas d'erreur)
int connect_to_server_4(void);

// Crée une connexion IPv6 avec le serveur et renvoie la socket associée (-1 en cas d'erreur)
int connect_to_server_6(void);

// Envoie une requête d'inscription pour le pseudo donné.
// Retourne l'id attribué par le serveur en cas de succès, -1 sinon.
int inscription(char *pseudo);

// Poste un billet sur le serveur renvoie 0 en cas de succès, -1 sinon
int poster_billet(int id);

// Demander la liste des n derniers billets
int demander_billets(int id);

// Lit l'entrée standard, retourne l'entier positif correspondant, -1 si pas positif ou pas un entier
int read_int(void);

#endif
