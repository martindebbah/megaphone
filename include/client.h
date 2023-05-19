#ifndef CLIENT_H
#define CLIENT_H

// Crée une connexion avec le serveur et renvoie la socket associée (-1 en cas d'erreur)
int connect_to_server(void);

// Envoie une requête d'inscription pour le pseudo donné.
// Retourne l'id attribué par le serveur en cas de succès, -1 sinon.
int inscription(char *pseudo);

// Poste un billet sur le serveur renvoie 0 en cas de succès, -1 sinon
int poster_billet(int id);

// Demander la liste des n derniers billets
int demander_billets(int id);

// Ajoute un fichier sur le serveur, renvoie 0 en cas de succès, -1 sinon
int ajouter_fichier(void);

// Télécharge un fichier disponible sur le serveur, renvoie 0 en cas de succès, -1 sinon
int telecharger_fichier(void);

// Lit l'entrée standard, retourne l'entier positif correspondant, -1 si pas positif ou pas un entier
int read_int(void);

#endif
