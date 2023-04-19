#ifndef CLIENT_H
#define CLIENT_H

// Crée une connexion avec le serveur et renvoie la socket associée (-1 en cas d'erreur)
int connect_to_server(void);

// Envoie une requête d'inscription pour le pseudo donné.
// Retourne l'id attribué par le serveur en cas de succès, -1 sinon.
int inscription(char *pseudo);

// Poste un billet sur le serveur renvoie 0 en cas de succès, 1 sinon
int poster_billet(int sock, int id, int numfil, char *data, int datalen);

// Demander la liste des n derniers billets
int demander_billets(int sock, int id, int numfil, int n);

#endif
