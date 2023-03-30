# include "../include/megaphone.h"

// Pour lancer le programme on lance `client.out`
// Et on fait `netcat -l 7777`

// On travaille sur lulu
#define ADDR "192.168.70.236"
// Port défini par le sujet
#define PORT 7777

int main(void) {
    // inscription
    int id = inscription("Martin");
    if (id == -1)
        exit(1);

    printf("Votre numéro d'identification est: %d\n", id);

    // `echo $?` pour valeur de retour
    exit(0);
}

int connect_to_server() {
    // on crée la socket client
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Erreur socket");
        return -1;
    }
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    // En IPv4 du coup
    addr.sin_family = AF_INET;
    // On transforme en big endian le numéro du port
    addr.sin_port = htons(PORT);

    // affecte à `dst` l’adresse au format réseau (entier codé en big-endian) 
    // correspondant à l’adresse représentée par la chaîne de caractères `src`.
    if (inet_pton(AF_INET, ADDR, &addr.sin_addr) != 1) {
        close(sock);
        perror("Erreur inet_pton");
        return -1;
    }

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        close(sock);
        perror("Erreur connexion");
        return -1;
    }

    return sock;
}

int inscription(char *pseudo) {
    // Création de la struct new_client avec le pseudo donné
    new_client_t *new_client = create_new_client(pseudo);
    if (!new_client)
        goto error;
    
    // Connection au serveur
    int sock = connect_to_server();
    if (sock < 0)
        goto error;
    
    // Envoie de la requête d'inscription
    if (send_new_client(sock, new_client) != 0)
        goto error;

    // Lecture de la réponse du serveur
    server_message_t *server_message = read_server_message(sock);
    if (!server_message)
        goto error;

    // Si le serveur renvoie une erreur
    if (server_message -> codereq == 31)
        goto error;

    // Sauvegarde de l'id envoyé par le serveur
    int id = server_message -> id;

    // Libère la mémoire allouée par la fonction
    delete_new_client(new_client);
    delete_server_message(server_message);
    close(sock);

    // Retourne l'id sauvegardé
    return id;

    error:
        if (new_client)
            delete_new_client(new_client);
        if (server_message)
            delete_server_message(server_message);
        if (sock >= 0)
            close(sock);
        return -1;
}

int poster_billet(int sock, int id, int numfil, char *data, int datalen){
    // on crée le billet
    client_message_t *billet = create_client_message(2, id, numfil, 0, datalen, data);
    if(billet == NULL){
        perror("Erreur création billet");
        return 1;
    }

    // on envoie le billet
    if(send_client_message(sock, billet) != 0){
        perror("Erreur envoi billet");
        return 1;
    }

    // message du serveur

    return 0;
}

int demander_billets(int sock, int id, int numfil, int n){
    // on crée le billet
    client_message_t *billet = create_client_message(3, id, numfil, n, 0, "");
    if(billet == NULL){
        perror("Erreur création billet");
        return 1;
    }

    // on envoie le billet
    if(send_client_message(sock, billet) != 0){
        perror("Erreur envoi billet");
        return 1;
    }

    // message du serveur

    return 0;    
}
