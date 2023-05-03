# include "../include/megaphone.h"

// Pour lancer le programme : `./client`

// Identifiant de l'utilisateur
static int id = -1;

int main(void) {
    char ans[MAX_MESSAGE_SIZE] = {0};
    int notLogged = 1;
    while (notLogged) { // Boucle tant que pas d'identitfiant
        printf("Êtes-vous déjà inscrit ? (o/n/q[uitter])\n");
        int nread = read(0, ans, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("Erreur read demande inscription");
            return 1;
        }
        printf("\n");

        if (ans[0] == 'o') { // Déjà inscrit
            memset(ans, 0, MAX_MESSAGE_SIZE);
            int idNotValid = 1;
            while (idNotValid) { // Boucle tant que l'id n'est pas un entier
                printf("Veuillez entrer votre identifiant\n");
                nread = read(0, ans, MAX_MESSAGE_SIZE);
                if (nread == -1) {
                    perror("Erreur read identifiant");
                    return 1;
                }
                printf("\n");

                char *endptr;
                id = strtol(ans, &endptr, 10);

                if (endptr == ans || id < 0) { // Valeur invalide
                    printf("L'identifiant n'est pas valide, un entier positif est attendu\n");
                }else {
                    idNotValid = 0;
                }
                memset(ans, 0, MAX_MESSAGE_SIZE);
            }
            notLogged = 0;
        }else if (ans[0] == 'n') { // Pas encore inscrit
            memset(ans, 0, MAX_MESSAGE_SIZE);
            printf("Veuillez entrer votre pseudo\n");
            nread = read(0, ans, MAX_MESSAGE_SIZE);
            if (nread == -1) {
                perror("Erreur read pseudo");
                return 1;
            }
            ans[nread - 1] = 0;
            printf("\n");

            id = inscription(ans);
            if (id == -1)
                printf("Une erreur est survenue, veuillez réessayer\n");
            else
                notLogged = 0;
        }else if (ans[0] == 'q') { // Quitter
            notLogged = 0;
        }else {
            printf("Veuillez entrer `o` pour oui ou `n` pour non\n");
            printf("`q` pour quitter l'application\n");
            memset(ans, 0, MAX_MESSAGE_SIZE);
        }
    }

    if (id == -1) {
        printf("Au revoir\n");
        exit(0);
    }

    memset(ans, 0, MAX_MESSAGE_SIZE);
    printf("Votre numéro d'identification est: %d\n", id);

    int action = 1;
    while(action){
        printf("Que voulez-vous faire ?\n");
        printf("1. Poster un billet (p)\n");
        printf("2. Lister les n derniers billets (l)\n");
        printf("4. Ajouter un fichier (a)\n");
        printf("3. Quitter (q)\n");

        int nread = read(0, ans, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("Erreur read action");
            return 1;
        }
        printf("\n");

        int err = 0;
        switch (ans[0]) {
            case 'p':
                err = poster_billet(id);
                break;
            case 'l':
                err = demander_billets(id);
                break;
            case 'a':
                err = ajouter_fichier();
                break;
            case 'q':
                action = 0;
                break;
            default:
                printf("Veuillez entrer `p` pour poster un billet ou `l` pour lister les billets\n");
                printf("ou `q` pour quitter l'application.\n");
                break;
        }

        if (err) {
            printf("Erreur\n");
        }
        memset(ans, 0, MAX_MESSAGE_SIZE);
    }

    printf("Déconnexion réussie !\n");

    // `echo $?` pour valeur de retour
    exit(0);
}

int connect_to_server(void) {
    if (TYPE == 4) // IPv4
        return connect_to_server_4();
    if (TYPE == 6) // IPv6
        return connect_to_server_6();
    // Erreur
    return -1;
}

int connect_to_server_4(void) {
    // Socket client
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("Erreur socket IPv4");
        return -1;
    }

    // Structure adresse IPv4
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ADDR4, &addr.sin_addr) != 1) {
        perror("Erreur inet_pton IPv4");
        close(sock);
        return -1;
    }

    // Connexion
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        perror("Erreur connexion IPv4");
        close(sock);
        return -1;
    }

    return sock;
}

int connect_to_server_6(void) {
    // Socket client
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur socket IPv6");
        return -1;
    }

    // Structure adresse IPv6
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);

    if (inet_pton(AF_INET6, ADDR6, &addr.sin6_addr) != 1) {
        perror("Erreur inet_pton IPv6");
        close(sock);
        return -1;
    }

    // Connexion
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        perror("Erreur connexion IPv6");
        close(sock);
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
    char *data = calloc(1, sizeof(char) * (MAX_MESSAGE_SIZE + 1));
    client_message_t *billet = NULL;
    server_message_t *mes = NULL;
    char ans[MAX_MESSAGE_SIZE] = {0};

    int post = 1;
    printf("Veuillez entrer le numéro du fil.\n");
    while(post){
        int nread = read(0, ans, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("read numfil");
            goto error;
        }
        printf("\n");

        char *endptr;
        numfil = strtol(ans, &endptr, 10);
        if (ans == endptr || numfil < 0) { // Valeur invalide
            printf("Veuillez entrer un entier positif !\n");
        }else {
            post = 0;
        }

        memset(ans, 0, MAX_MESSAGE_SIZE);
    }
    
    post = 1;
    printf("Veuillez entrer le message à poster.\n");
    while(post){
        int nread = read(0, data, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("Erreur read message");
            goto error;
        }
        printf("\n");

        if (nread > MAX_MESSAGE_SIZE) { // N'arrivera pas puisqu'on lit max MAX_MESSAGE_SIZE caractères
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
    billet = create_client_message(2, id, numfil, 0, datalen, data);
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
    mes = read_server_message(sock);
    if(mes == NULL){
        perror("Erreur lecture message serveur");
        goto error;
    }

    printf("Code requête : %d\n", mes -> codereq);
    printf("Id : %d\n", mes -> id);
    if(mes -> codereq == 2){
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
    client_message_t *billet;
    server_message_t *mes = NULL;

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
    billet = create_client_message(3, id, numfil, n_billets, 0, "");
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
    mes = read_server_message(sock);
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

int ajouter_fichier(void) {
    client_message_t *client_message = NULL;
    server_message_t *server_message = NULL;
    int file = -1;
    int sock = -1;
    int udp_sock = -1;
    char buf[MAX_MESSAGE_SIZE] = {0};
    char filename[MAX_MESSAGE_SIZE] = {0};

    int notValid = 1;
    while (notValid) {
        printf("Veuillez entrer le chemin vers le fichier que vous voulez ajouter\n");
        int nread = read(0, buf, MAX_MESSAGE_SIZE);
        if (nread < 0) {
            perror("read nom fichier");
            goto error;
        }
        buf[nread - 1] = 0;
        printf("\n -> :%s:\n", buf);

        file = open(buf, O_RDONLY);
        if (file < 0) {
            printf("Référence invalide\n");
        }else {
            memmove(filename, buf, strlen(buf));
            notValid = 0;
        }
        memset(buf, 0, MAX_MESSAGE_SIZE);
    }

    notValid = 1;
    int numfil = -1;
    while (notValid) {
        printf("Sur quel fil voulez-vous ajouter ce fichier ?\n");
        // numfil = read_int();
        int nread = read(0, buf, MAX_MESSAGE_SIZE);
        if (nread < 0) {
            perror("read_entier");
            goto error;
        }
        printf("\n");

        char *endptr;
        numfil = strtol(buf, &endptr, 10);
        
        if (buf == endptr || numfil < 0) {
            printf("Veuillez entrer un entier positif !\n");
        }else {
            notValid = 0;
        }
        memset(buf, 0, MAX_MESSAGE_SIZE);
    }

    sock = connect_to_server();
    if (sock < 0) {
        perror("connect ajouter_fichier");
        goto error;
    }

    client_message = create_client_message(5, id, numfil, 0, strlen(filename), filename);
    if (!client_message) {
        perror("client_message ajouter_fichier");
        goto error;
    }

    // Envoi du message client
    if (send_client_message(sock, client_message) < 0) {
        perror("send client ajouter_fichier");
        goto error;
    }

    // Réception du message du serveur
    server_message = read_server_message(sock);
    if (!server_message) {
        perror("server message ajouter_fichier");
        goto error;
    }

    // Erreur
    if (server_message -> codereq != 5)
        goto error;

    // Création socket UDP
    udp_sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("socket UDP");
        goto error;
    }

    int port = ntohs(server_message -> nb);
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    if (inet_pton(AF_INET6, ADDR6, &addr.sin6_addr) != 1) {
        perror("inet_pton ajouetr_fichier");
        goto error;
    }

    char buffer[UDP_PACKET_SIZE] = {0};
    int reading = 1;
    uint16_t numblock = 1;

    while (reading) {
        int nread = read(file, buffer, UDP_PACKET_SIZE);
        if (nread < 0) {
            perror("read file");
            goto error;
        }

        if (nread == 0) { // On est à la fin du fichier
            reading = 0;
        }else {
            udp_t *udp = create_udp(id, numblock, buffer);
            if (!udp) {
                perror("create_udp ajouter_fichier");
                goto error;
            }

            if (send_udp(udp_sock, addr, udp) != 0) {
                perror("send udp ajouter_fichier");
                delete_udp(udp);
                goto error;
            }

            delete_udp(udp);
            numblock++;
        }
        memset(buffer, 0, UDP_PACKET_SIZE);
    }

    // Lire une dernière réponse du serveur ?

    // Libération de la mémoire allouée
    delete_client_message(client_message);
    delete_server_message(server_message);
    close(file);
    close(sock);
    close(udp_sock);

    return 0;

    error:
        if (client_message)
            delete_client_message(client_message);
        if (server_message)
            delete_server_message(server_message);
        if (file >= 0)
            close(file);
        if (sock >= 0)
            close(sock);
        if (udp_sock >= 0)
            close(udp_sock);
        return -1;
}
