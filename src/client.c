# include "../include/megaphone.h"

// Pour lancer le programme on lance `client.out`
// Et on fait `netcat -l 7777`

// On travaille sur lulu
#define ADDR "192.168.70.236"
// Port défini par le sujet
#define PORT 7777

// poste un billet sur le serveur renvoie 0 en cas de succès, 1 sinon
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

// demander la liste des n derniers billets
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

int main(void) {
    // on crée la socket client
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Erreur socket");
        exit(1);
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
        perror("Erreur inet_pton");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        perror("Erreur connexion");
        exit(1);
    }

    // inscription
    new_client_t *new_client = create_new_client("Martin");
    delete_new_client(new_client);

    close(sock);
    // `echo $?` pour valeur de retour
    exit(0);
}
