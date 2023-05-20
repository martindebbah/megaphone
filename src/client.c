# include "../include/megaphone.h"

// Pour lancer le programme : `./client`

// Identifiant de l'utilisateur
static int id = -1;
char *hostname;
char hostaddr[INET6_ADDRSTRLEN] = {0};
static pthread_t readInputThreadId;
static int sock_udp = 0;
static int quit = 1;

int main(int argc, char **argv) {
    // hostname
    if (argc < 2) {
        printf("Il manque le nom de domaine\n");
        return 1;
    }
    hostname = argv[1];

    char buf[MAX_MESSAGE_SIZE] = {0};
    int notLogged = 1;
    while (notLogged) { // Boucle tant que pas d'identitfiant
        printf("Êtes-vous déjà inscrit ? (o/n/q[uitter])\n");
        int nread = read(0, buf, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("Erreur read demande inscription");
            return 1;
        }
        printf("\n");

        if (buf[0] == 'o') { // Déjà inscrit
            int idNotValid = 1;
            while (idNotValid) { // Boucle tant que l'id n'est pas un entier strictement positif
                printf("Veuillez entrer votre identifiant\n");
                id = read_int();
                if (id > 0)
                    idNotValid = 0;
                else
                    printf("Identifiant 0 non valide\n");
            }
            notLogged = 0;

        }else if (buf[0] == 'n') { // Pas encore inscrit
            memset(buf, 0, MAX_MESSAGE_SIZE);
            printf("Veuillez entrer votre pseudo\n");
            nread = read(0, buf, MAX_MESSAGE_SIZE);
            if (nread == -1) {
                perror("Erreur read pseudo");
                return 1;
            }
            buf[nread - 1] = 0;
            printf("\n");

            id = inscription(buf);
            if (id == -1)
                printf("Une erreur est survenue, veuillez réessayer\n");
            else
                notLogged = 0;

        }else if (buf[0] == 'q') { // Quitter
            notLogged = 0;
        }else {
            printf("Veuillez entrer `o` pour oui ou `n` pour non\n");
            printf("`q` pour quitter l'application\n");
            memset(buf, 0, MAX_MESSAGE_SIZE);
        }
    }

    // L'utilisateur quitte l'application
    if (id == -1) {
        printf("Au revoir\n\n");
        exit(0);
    }

    // memset(buf, 0, MAX_MESSAGE_SIZE);
    // printf("Votre numéro d'identification est: %d\n", id);
    // printf("Veuillez noter ce numéro pour pouvoir vous reconnecter plus tard\n\n");

    // int action = 1;
    // while(action){
    //     printf("Que voulez-vous faire ?\n");
    //     printf("1. Poster un billet (p)\n");
    //     printf("2. Lister les n derniers billets (l)\n");
    //     printf("4. Ajouter un fichier (a)\n");
    //     printf("5. Télécharger un fichier (t)\n");
    //     printf("3. Quitter (q)\n");

    //     int nread = read(0, buf, MAX_MESSAGE_SIZE);
    //     if (nread == -1) {
    //         perror("Erreur read action");
    //         return 1;
    //     }
    //     printf("\n");

    //     int err = 0;
    //     switch (buf[0]) {
    //         case 'p':
    //             err = poster_billet(id);
    //             break;
    //         case 'l':
    //             err = demander_billets(id);
    //             break;
    //         case 'a':
    //             err = ajouter_fichier();
    //             break;
    //         case 't':
    //             err = telecharger_fichier();
    //             break;
    //         case 'q':
    //             action = 0;
    //             break;
    //         default:
    //             printf("Veuillez entrer `p` pour poster un billet ou `l` pour lister les billets\n");
    //             printf("ou `a` pour ajouter un fichier ou `t` pour télécharger un fichier\n");
    //             printf("ou `q` pour quitter l'application.\n");
    //             break;
    //     }

    //     if (err) {
    //         printf("Une erreur est survenue\n\n");
    //     }
    //     memset(buf, 0, MAX_MESSAGE_SIZE);
    // }

    // printf("Pour pouvoir vous reconnecter, n'oubliez pas de noter votre numéro d'identification: %d\n", id);
    // printf("Déconnexion réussie !\n\n");

    // // `echo $?` pour valeur de retour
    // exit(0);
	action(NULL);
}

void handler(int sig) {
	pthread_join(readInputThreadId, NULL);
	if (sock_udp != 0)
		close(sock_udp);
	quit = 0;
}

void *action(void *arg) {
	char buf[MAX_MESSAGE_SIZE] = {0};
	int action = 1;
    while(action && quit){
        printf("Que voulez-vous faire ?\n");
        printf("1. Poster un billet (p)\n");
        printf("2. Lister les n derniers billets (l)\n");
		printf("3. Abonnement a un fil (s)\n");
        printf("4. Ajouter un fichier (a)\n");
        printf("5. Télécharger un fichier (t)\n");
        printf("6. Quitter (q)\n");

        int nread = read(0, buf, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("Erreur read action");
            return NULL;
        }
        printf("\n");

        int err = 0;
        switch (buf[0]) {
            case 'p':
                err = poster_billet(id);
                break;
            case 'l':
                err = demander_billets(id);
                break;
			case 's':
				err = abonnement_billets(id);
				break; 
            case 'a':
                err = ajouter_fichier();
                break;
            case 't':
                err = telecharger_fichier();
                break;
            case 'q':
				printf("Pour pouvoir vous reconnecter, n'oubliez pas de noter votre numéro d'identification: %d\n", id);
				printf("Déconnexion réussie !\n\n");

				// `echo $?` pour valeur de retour
				quit = 0;

				return NULL;
            default:
                printf("Veuillez entrer `p` pour poster un billet ou `l` pour lister les billets\n");
                printf("ou `a` pour ajouter un fichier ou `t` pour télécharger un fichier\n");
                printf("ou `q` pour quitter l'application.\n");
                break;
        }

        if (err) {
            printf("Une erreur est survenue\n\n");
        }
		else if (err == 113) {
			return NULL;
		}
        memset(buf, 0, MAX_MESSAGE_SIZE);
    }

	return NULL;
}

int connect_to_server(void) {
    int sock = -1;
    struct addrinfo hints, *r, *p;
    memset(&hints, 0, sizeof(hints));
    // Connexion IPv4 ou IPv6
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, PORT_STR, &hints, &r) != 0 || !r)
        return -1;

    p = r;

    while (p) {
        if ((sock = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) > 0) {
            if (connect(sock, p -> ai_addr, p -> ai_addrlen) == 0)
                break;

            close(sock);
            sock = -1;
        }
        p = p -> ai_next;
    }

    if (p) { // On enregistre l'adresse pour socket UDP
        if (p -> ai_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *) p -> ai_addr;
            inet_ntop(AF_INET, &(addr -> sin_addr), hostaddr, INET_ADDRSTRLEN);
        }else {
            struct sockaddr_in6 *adr6 = (struct sockaddr_in6 *) p -> ai_addr;
            inet_ntop(AF_INET6, &(adr6->sin6_addr), hostaddr, INET6_ADDRSTRLEN);
        }
    }

    freeaddrinfo(r);

    return sock;
}

int inscription(char *pseudo) {
    // Initialisation des pointeurs
    new_client_t *new_client = NULL;
    server_message_t *server_message = NULL;
    int sock = -1;

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

    int post = 1;
    while(post){
        printf("Veuillez entrer le numéro du fil.\n");
        numfil = read_int();
        if (numfil != -1)
            post = 0;
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

    // printf("Code requête : %d\n", mes -> codereq);
    // printf("Id : %d\n", mes -> id);
    if(mes -> codereq == 2) {
        printf("Message posté sur le fil %d !\n\n", mes -> numfil);
    }else {
        goto error;
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
    client_message_t *billet = NULL;
    server_message_t *mes = NULL;

    int post = 1;
    while(post){
        printf("Veuillez entrer le numéro du fil.\n");
        printf("Pour demander tous les fils, entrez 0.\n");
        numfil = read_int();
        if (numfil != -1)
            post = 0;
    }

    post = 1;
    while(post){
        printf("Veuillez entrer le nombre de messages.\n");
        printf("Pour demander tous les messages, entrez 0.\n");
        n_billets = read_int();
        if (n_billets != -1)
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

    if (mes -> codereq != 3)
        goto error;

    for(int i = 0; i < mes->nb; i++){ // le nombre de billets que va envoyer le serveur
        post_t *posts = read_post(sock);
        if(posts == NULL){
            perror("Erreur lecture billet");
            goto error;
        }
        remove_hash(posts -> origin);
        remove_hash(posts -> pseudo);
        printf("Fil : %d\n", posts->numfil);
        printf("Initiateur du fil : %s\n", posts->origin);
        printf("Auteur du message : %s\n", posts->pseudo);
        char data[posts -> datalen + 1];
        memmove(data, posts -> data, posts -> datalen);
        data[posts -> datalen] = 0;
        printf("Billet : %s\n", data);
        printf("\n");
        delete_post(posts);
    }

    if (mes -> nb == 0)
        printf("Il n'y a pas de message\n\n");

    delete_client_message(billet);
    delete_server_message(mes);
    close(sock);

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
    char file_path[MAX_MESSAGE_SIZE] = {0};

    int notValid = 1;
    while (notValid) {
        printf("Veuillez entrer le chemin vers le fichier que vous voulez ajouter\n");
        int nread = read(0, file_path, MAX_MESSAGE_SIZE);
        if (nread < 0) {
            perror("read nom fichier");
            goto error;
        }
        printf("\n");
        file_path[nread - 1] = 0;

        file = open(file_path, O_RDONLY);
        if (file < 0) {
            printf("La référence n'est pas valide\n");
            memset(file_path, 0, MAX_MESSAGE_SIZE);
        }else {
            notValid = 0;
        }
    }

    // On récupère le nom du fichier
    char *filename = strrchr(file_path, '/');
    if (filename)
        filename++;
    else
        filename = file_path;

    notValid = 1;
    int numfil = -1;
    while (notValid) {
        printf("Sur quel fil voulez-vous ajouter ce fichier ?\n");
        numfil = read_int();
        if (numfil >= 0)
            notValid = 0;
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
    if (send_client_message(sock, client_message) != 0) {
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

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(server_message -> nb);
    if (inet_pton(AF_INET6, hostaddr, &addr.sin6_addr) != 1) {
        perror("inet_pton ajouter_fichier");
        goto error;
    }

    if (send_file(udp_sock, addr, file, id) != 0) {
        perror("send ajouter_fichier");
        goto error;
    }

    delete_server_message(server_message);
    server_message = read_server_message(sock);
    if (!server_message) {
        perror("dernier read server_message ajouter_fichier");
        goto error;
    }

    if (server_message -> codereq != 5)
        goto error;

    printf("Le fichier %s a été ajouté au fil %d\n\n", filename, server_message -> numfil);

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

int telecharger_fichier(void) {
    int notValid = 1;
    int numfil = 0;
    while (notValid) {
        printf("Entrez le numéro du fil depuis lequel vous souhaitez télécharger le fichier\n");
        numfil = read_int();
        if (numfil > 0)
            notValid = 0;
        
        if (numfil == 0)
            printf("Le fil numéro 0 est invalide\n");
    }

    char filename[MAX_MESSAGE_SIZE] = {0};
    printf("Entrez le nom du fichier\n");
    int nread = read(0, filename, MAX_MESSAGE_SIZE);
    if (nread < 0) {
        perror("read telecharger_fichier");
        return -1;
    }
    printf("\n");
    filename[nread - 1] = 0; // Pour le \n

    // Initialisation pointeurs
    int sock = -1;
    int udp_sock = -1;
    client_message_t *client_message = NULL;
    server_message_t *server_message = NULL;
    file_t *file = NULL;

    // Socket TCP
    sock = connect_to_server();
    if (sock < 0) {
        perror("connect telecharger_fichier");
        goto error;
    }

    // Initialisation du socket UDP en IPv6
    udp_sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("socket UDP telecharger_fichier");
        goto error;
    }

    // Structure adresse IPv6
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(UDP_PORT);
    addr.sin6_addr = in6addr_any;

    // Bind
    if (bind(udp_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind udp socket");
        goto error;
    }

    file = create_file(filename, nread - 1);
    if (!file) {
        perror("création file");
        goto error;
    }

    // Envoi message client
    client_message = create_client_message(6, id, numfil, UDP_PORT, nread - 1, filename);
    if (!client_message) {
        perror("création client_message telecharger_fichier");
        goto error;;
    }

    if (send_client_message(sock, client_message) != 0) {
        perror("send telecharger_fichier");
        goto error;
    }

    // Réception réponse serveur
    server_message = read_server_message(sock);
    if (!server_message) {
        perror("read telecharger_fichier");
        goto error;
    }

    // Réponse négative du serveur
    if (server_message -> codereq != 6)
        goto error;

    // Réception du fichier sur socket UDP
    if (recv_file(udp_sock, file) != 0) {
        perror("recv_file");
        goto error;
    }

    // Téléchargement du fichier
    const char *path = "telechargement";
    DIR *dir = opendir(path);
    if (!dir) {
        if (mkdir(path, 0755) != 0) {
            perror("Création répertoire");
            goto error;
        }
    }else {
        closedir(dir);
    }

    char file_path[270] = {0};
    snprintf(file_path, 270, "%s/%s", path, filename);

    if (write_file(file_path, file) != 0) {
        perror("Ecriture fichier");
        goto error;
    }

    printf("Le fichier %s a bien été téléchargé, vous le retrouverez dans le dossier `telechargement`\n\n", filename);

    // Libération des pointeurs
    close(sock);
    close(udp_sock);
    delete_client_message(client_message);
    delete_server_message(server_message);
    delete_file(file);

    return 0;

    error:
        if (sock >= 0)
            close(sock);
        if (udp_sock >= 0)
            close(udp_sock);
        if(client_message)
            delete_client_message(client_message);
        if (server_message)
            delete_server_message(server_message);
        if (file)
            delete_file(file);
        return -1;
}

int read_int(void) {
    char buf[MAX_MESSAGE_SIZE] = {0};
    int nread = read(0, buf, MAX_MESSAGE_SIZE);
    if (nread < 0) {
        perror("read_entier");
        return -1;
    }
    printf("\n");

    char *endptr;
    int n = strtol(buf, &endptr, 10);
    
    if (buf == endptr || n < 0) {
        printf("Veuillez entrer un entier positif !\n");
        return -1;
    }
    return n;
}

int abonnement_billets(int id) {
	int sock = connect_to_server();
	if (sock < 0) {
        perror("connect ajouter_fichier");
        return -1;
    }
	int numfil = 0;
	char ans[MAX_MESSAGE_SIZE] = {0};

    int post = 1;
    printf("Veuillez entrer le numéro du fil.\n");
    while(post){
        int nread = read(0, ans, MAX_MESSAGE_SIZE);
        if (nread == -1) {
            perror("read numfil");
            return -1;
        }
        printf("\n");
        char *endptr;
        numfil = strtol(ans, &endptr, 10);
        if (ans == endptr || numfil < 0) { // Valeur invalide
            printf("Veuillez entrer un entier positif !\n");
        } else {
            post = 0;
        }
        memset(ans, 0, MAX_MESSAGE_SIZE);
    }

	client_message_t *billet = create_client_message(4, id, numfil, 0, 0, "");
    if(billet == NULL){
        perror("Erreur création billet");
        return -1;
    }

    // on envoie le billet
    if(send_client_message(sock, billet) != 0) {
        perror("ici Erreur envoi billet");
		return -1;
    }
	delete_client_message(billet);
	subscribe_t *mess = read_subscribe_message(sock);
	if (!mess) {
		perror("Erreur pendant le read_subscribe");
		return -1;
	}

	if (mess -> server_message -> codereq != 4) {
		printf("Pas le bon message recu par read_subscribe\n");
		delete_subscribe_message(mess);
		close(sock);
		return -1;
	}

	char **split = calloc(8, sizeof(char *));
	
	for (int i = 0; i < 8; i++)
    	split[i] = to_hex_string(mess -> addrmult[i]);
	delete_subscribe_message(mess);

	char *ip = "";
	for (int i = 0; i < 8; i++)
	{
		ip  = ft_strjoin(ip, split[i]);
		free(split[i]);
		if (i != 7)
			ip = ft_strjoin(ip, ":");
	}

	free(split);

	int socket_udp = socket(AF_INET6, SOCK_DGRAM, 0);
	sock_udp = socket_udp;
	if (socket_udp < 0) {
		perror("socket");
		return -1;
	}
	int yes = 1;
	if (setsockopt(socket_udp, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("setsockopt reuse");
		return -1;
	}

	struct ipv6_mreq group = {0};
	
    if (inet_pton(AF_INET6, ip, &group.ipv6mr_multiaddr) <= 0) {
        perror("inet_pton");
		free(ip);
        return -1;
    }
	free(ip);

	unsigned int ifindex = if_nametoindex("eth0");
    if (!ifindex) {
        perror("if_nametoindex");
    }

	// permet de l'abonner
	group.ipv6mr_interface = ifindex;
    if (setsockopt(socket_udp, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group)) < 0) {
        perror("setsockopt");
		close(socket_udp);
        return -1;
    }

	struct sockaddr_in6 addr = {0};
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(PORT);
	int nbind = bind(socket_udp, (struct sockaddr *)&addr, sizeof(addr));
	if (nbind < 0) {
		perror("bind");
		close(socket_udp);
		return 1;
	}


	printf("Abonnement avec succes sur le fil : %d\n", numfil);

	close(sock);

    pthread_create(&readInputThreadId, NULL, action, &socket_udp);
	if (quit == 0) {
		close(socket_udp);
		pthread_join(readInputThreadId, NULL);
		return 0;
	}
	
    fd_set readfds;
	notification_t *buffer = NULL;
	char notification[1024] = {0};
	char buf[1024] = {0};
	int pos = 0;

	while (quit == 1) {
        FD_ZERO(&readfds);
        FD_SET(socket_udp, &readfds);

        // Définir le timeout pour select
		int max = 0;
		char tmp[1024] = {0};
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        // Utiliser select pour attendre des données en lecture
        int readyDescriptors = select(socket_udp + 1, &readfds, NULL, NULL, &timeout);
        if (readyDescriptors == -1) {
            perror("select");
			pthread_join(readInputThreadId, NULL);
            return 1;
        }
		else if (readyDescriptors > 0 && FD_ISSET(socket_udp, &readfds)) {
			// Le socket client a des données disponibles pour la lecture
			buffer = read_notification_message(socket_udp);
			
			if (!buffer) {
				perror("read in the select");
				pthread_join(readInputThreadId, NULL);
				return 1;
			}
			else {
				remove_hash(buffer -> pseudo);
				sprintf(buf, "Il y a un %s sur le fil %d par %s\n", buffer -> data, buffer -> numfil, buffer -> pseudo);
				int	bytesRead = strlen(buf);
				if (pos + bytesRead < 1023) {
					memmove(notification + pos, buf, bytesRead);
					pos += bytesRead;
					delete_notification_message(buffer);
				}
				else {
					max = 1;
					memmove(tmp, buf, bytesRead);
					delete_notification_message(buffer);
				}
			}
        } else if (readyDescriptors == 0 || max == 1){
			if(notification[0] != '\0') {
				printf("%s\n",notification);
				bzero(notification, 1024);
				pos = 0;
                if (tmp[0] != '\0') {
					memmove(notification, tmp, strlen(tmp));
					pos += strlen(tmp);
				}
			}
        }
    }
	pthread_join(readInputThreadId, NULL);

	close(socket_udp);
	return 0;
}

