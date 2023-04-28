# include "../include/megaphone.h"

// Pour lancer le programme on lance `./client`

// On travaille en localhost
#define ADDR "127.0.0.1"
#define PORT 30000

// Taille maximale d'un post
#define MAX_MESSAGE_SIZE 50

// Identifiant de l'utilisateur
static int id = -1;

int main(void) {
    char *ans = calloc(10, 1);
    int notLogged = 1;
    while (notLogged) { // Boucle tant que pas d'identitfiant
        printf("Êtes-vous déjà inscrit ? (o/n)\n");
        int nread = read(0, ans, 10);
        if (nread == -1) {
            perror("Erreur read demande inscription");
            goto error;
        }

        if (ans[0] == 'o') { // Déjà inscrit
            int idNotValid = 1;
            while (idNotValid) { // Boucle tant que l'id n'est pas un entier
                printf("Veuillez entrer votre identifiant\n");
                nread = read(0, ans, 10);
                if (nread == -1) {
                    perror("Erreur read identifiant");
                    goto error;
                }

                char *endptr;
                id = strtol(ans, &endptr, 10);

                if (endptr == ans || id < 0) { // Valeur invalide
                    printf("L'identifiant n'est pas valide, un entier positif est attendu\n");
                }else {
                    idNotValid = 0;
                    notLogged = 0;
                }
            }
            notLogged = 0;
        }else if (ans[0] == 'n') { // Pas encore inscrit
            printf("Veuillez entrer votre pseudo\n");
            nread = read(0, ans, 10);
            if (nread == -1) {
                perror("Erreur read pseudo");
                goto error;
            }

            id = inscription(ans);
            notLogged = 0;
        }else { // Ni `o` ni `n`
            printf("Veuillez entrer `o` pour oui ou `n` pour non\n");
        }
    }

    if (id == -1)
        exit(1);

    printf("Votre numéro d'identification est: %d\n", id);

    int action = 1;
    printf("Que voulez-vous faire ?\n");
    printf("1. Poster un billet (p)\n");
    printf("2. Lister les n derniers billets (l)\n");
    while(action){
        int nread = read(0, ans, 10);
        if (nread == -1) {
            perror("Erreur read action");
            goto error;
        }

        if (ans[0] == 'p') {
            action = 0;
            int p = poster_billet(id);
            if(p == -1){
                perror("Erreur lors de l'envoi du billet");
                goto error;
            }
        }else if (ans[0] == 'l') {
            action = 0;
            //int l = demander_billets(id);
        }else {
            printf("Veuillez entrer `p` pour poster un billet ou `l` pour lister les billets\n");
        }
    }

    free(ans);

    // `echo $?` pour valeur de retour
    exit(0);

    error:
        if (ans)
            free(ans);
        return 1;
}

int connect_to_server(void) {
    // on crée la socket client
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0){
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
    // Initialisation des pointeurs
    new_client_t *new_client = NULL;
    int sock = -1;
    server_message_t *server_message = NULL;

    // Création de la struct new_client avec le pseudo donné
    new_client = create_new_client(pseudo);
    if (!new_client)
        goto error;
    
    // Connection au serveur
    sock = connect_to_server();
    if (sock < 0)
        goto error;
    
    // Envoie de la requête d'inscription
    if (send_new_client(sock, new_client) != 0)
        goto error;

    // Lecture de la réponse du serveur
    server_message = read_server_message(sock);
    if (!server_message)
        goto error;

    // TODO: Ajouter une condition pour CODEREQ = 31

    // Si le serveur renvoie une erreur
    if (server_message -> codereq != 1)
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

int poster_billet(int id){
    int sock = -1;
    int numfil = -1;
    int datalen = -1;
    char *data = malloc(sizeof(char) * (MAX_MESSAGE_SIZE + 1));

    int post = 1;
    printf("Veuillez entrer le numéro du fil.\n");
    while(post){
        scanf("%d", &numfil);
        if (numfil < 0)
            printf("Veuillez entrer un entier positif !\n");
        else
            post = 0;
    }
    post = 1;
    printf("Veuillez entrer le message à poster.\n");
    while(post){
        int nread = read(0, data, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("Erreur read message");
            return -1;
        }

        if (nread > MAX_MESSAGE_SIZE) {
            printf("Veuillez entrer un message de moins de %d caractères\n", MAX_MESSAGE_SIZE);
        }
        else {
            datalen = nread;
            data[nread] = 0;
            post = 0;
        }
    }

    // on crée la socket client
    sock = connect_to_server();
    if(sock < 0){
        perror("Erreur socket");
        goto error;
    }

    // on crée le billet
    client_message_t *billet = create_client_message(2, id, numfil, 0, datalen, data);
    if(billet == NULL){
        perror("Erreur création billet");
        goto error;
    }

    // on envoie le billet
    if(send_client_message(sock, billet) != 0){
        perror("Erreur envoi billet");
        goto error;
    }

    // message du serveur
    server_message_t *mes = read_server_message(sock);
    if(mes == NULL){
        perror("Erreur lecture message serveur");
        goto error;
    }

    printf("Code requête : %d\n", mes -> codereq);
    printf("Id : %d\n", mes -> id);
    if(mes -> codereq != 34){
        printf("Message posté !\n");
    }
    else{
        printf("Erreur lors de l'envoi du message\n");
    }

    free(data);
    delete_client_message(billet);
    delete_server_message(mes);
    close(sock);

    return 0;

    error: 
        if (data)
            free(data);
        if (billet)
            delete_client_message(billet);
        if (mes)
            delete_server_message(mes);
        if (sock >= 0)
            close(sock);
        return -1;
}

int demander_billets(int id){
    int sock = -1;
    int numfil = -1;
    int n_billets = -1;

    int post = 1;
    printf("Veuillez entrer le numéro du fil.\n");
    printf("Pour demander tous les fils, entrez 0.\n");
    while(post){
        scanf("%d", &numfil);
        if (numfil < 0)
            printf("Veuillez entrer un entier positif !\n");
        else
            post = 0;
    }
    post = 1;
    printf("Veuillez entrer le nombre de messages.\n");
    printf("Pour demander tous les messages, entrez 0.\n");
    while(post){
        scanf("%d", &n_billets);
        if (n_billets < 0)
            printf("Veuillez entrer un entier positif !\n");
        else
            post = 0;
    }

    // on crée la socket client
    sock = connect_to_server();
    if(sock < 0){
        perror("Erreur socket");
        return -1;
    }

    // on crée le billet
    client_message_t *billet = create_client_message(3, id, numfil, n_billets, 0, "");
    if(billet == NULL){
        perror("Erreur création billet");
        goto error;
    }

    // on envoie le billet
    if(send_client_message(sock, billet) != 0){
        perror("Erreur envoi billet");
        goto error;
    }

    // message du serveur
    server_message_t *mes = read_server_message(sock);
    if(mes == NULL){
        perror("Erreur lecture message");
        goto error;
    }

    printf("Codereq : %d, Id : %d, Fil : %d\n", billet->codereq, billet->id, billet->numfil);

    for(int i = 0; i < mes->nb; i++){ // le nombre de billets que va envoyer le serveur
        post_t *posts = read_post(sock);
        if(posts == NULL){
            perror("Erreur lecture billet");
            goto error;
        }
        printf("Fil : %d\n", posts->numfil);
        printf("Origine : %s\n", posts->origin);
        printf("Pseudo : %s\n", posts->pseudo);
        printf("Billet : %s\n", posts->data);
        delete_post(posts);
    }

    return 0;

    error:
        if (billet)
            delete_client_message(billet);
        if (mes)
            delete_server_message(mes);
        if (sock >= 0)
            close(sock);
        return -1;    
}
