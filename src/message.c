#include "../include/megaphone.h"

uint16_t create_header(int codereq, int id) {
    // Initialisation des bits de l'en-tête à 0
    uint16_t header = 0;
    // Ajout de ID + décalage de 5 bits vers bit de poids fort
    header |= (uint16_t) id << 5;
    // Ajout de CODEREQ avec masque des 5 bits de poids faible (0x1F == 0b11111)
    header |= (uint16_t) (codereq & 0x1F);
    // Mise au format big-endian
    header = htons(header);

    return header;
}

new_client_t *create_new_client(char *pseudo) {
    new_client_t *new_client = calloc(1, sizeof(new_client_t));
    if (!new_client) {
        perror("alloc new_client");
        goto error;
    }

    // Création de l'en-tête (CODEREQ = 1, ID = 0)
    new_client -> codereq = 1;
    new_client -> id = 0;

    // Remplissage du PSEUDO
    int pseudo_len = strlen(pseudo);
    for (int i = 0; i < 10; i++) {
        if (i < pseudo_len)
            new_client -> pseudo[i] = pseudo[i];
        else
            new_client -> pseudo[i] = '#';
    }

    return new_client;

    error:
        if (new_client)
            delete_new_client(new_client);
        return NULL;
}

int send_new_client(int fd, new_client_t *new_client) {
    // Mise au format big-endian de l'en-tête
    uint16_t header = create_header(new_client -> codereq, new_client -> id);

    // Copie du message dans un tableau de char
    char data[12];
    memmove(&data[0], &header, 2);
    memmove(&data[2], &(new_client -> pseudo), 10);

    // Envoi du tableau
    if (send(fd, data, 12, 0) < 0) {
        perror("write new_client");
        return 1;
    }

    return 0;
}

new_client_t *read_to_new_client(char *data) {
    new_client_t *new_client = calloc(1, sizeof(new_client_t));
    if (!new_client) {
        perror("Erreur alloc new_client");
        goto error;
    }

    // Penser à la gestion d'erreur si message mal formatté

    uint16_t header = 0;
	memmove(&header, &data[0], 2);
	// Récupération du CODEREQ
	new_client -> codereq = ntohs(header) & 0x1F;
    // Récupération de l'ID
    new_client -> id = ntohs(header) >> 5;

    // Récupération du pseudo
    memmove(&(new_client -> pseudo), &data[2], 10);

    return new_client;

    error:
        if (new_client)
            delete_new_client(new_client);
        return NULL;
}

void delete_new_client(new_client_t *new_client) {
    if (new_client)
        free(new_client);
}

client_message_t *create_client_message(int codereq, int id, int numfil, int nb, int datalen, char *data) {
    client_message_t *client_message = calloc(1, sizeof(client_message_t));
    if (!client_message) {
        perror("alloc client_message");
        goto error;
    }

    client_message -> codereq = codereq;
    client_message -> id = id;
    client_message -> numfil = (uint16_t) numfil;
    client_message -> nb = (uint16_t) nb;
    client_message -> datalen = (uint8_t) datalen;

    // Copie de DATA
    client_message -> data = calloc(1, datalen);
    if (!client_message -> data) {
        perror("alloc data client_message");
        goto error;
    }
    memmove(client_message -> data, data, datalen);

    return client_message;

    error:
        if (client_message)
            delete_client_message(client_message);
        return NULL;
}

int send_client_message(int fd, client_message_t *client_message) {
    // Mise au format big-endian de l'en-tête
    uint16_t header = create_header(client_message -> codereq, client_message -> id);
    // Mise au format big-endian de NUMFIL
    uint16_t numfil = htons(client_message -> numfil);
    // Mise au format big-endian de NB
    uint16_t nb = htons(client_message -> nb);

    // Copie du client_message dans un tableau de char
    char data[7 + client_message -> datalen];
    memmove(&data[0], &header, 2);
    memmove(&data[2], &numfil, 2);
    memmove(&data[4], &nb, 2);
    memmove(&data[6], &(client_message -> datalen), 1);
    memmove(&data[7], client_message -> data, client_message -> datalen);

    // Envoi du tableau
    if (send(fd, data, 7 + client_message -> datalen, 0) < 0) {
        perror("write client_message");
        return 1;
    }

    return 0;
}

void delete_client_message(client_message_t *client_message) {
    if (!client_message)
        return;
    if (client_message -> data)
        free(client_message -> data);
    free(client_message);
}

server_message_t *create_server_message(int codereq, int id, int numfil, int nb) {
	server_message_t *server_message = calloc(1, sizeof(server_message_t));
	if (!server_message) {
        perror("alloc create_server_message");
        return NULL;
	}

    server_message -> codereq = codereq;
    server_message -> id = id;
    server_message -> numfil = numfil;
    server_message -> nb = nb;

	return (server_message);
}

int send_server_message(int fd, server_message_t *server_message) {
    // Mise au format big-endian de l'en-tête
	uint16_t header = create_header(server_message -> codereq, server_message -> id);
    // Mise au format big-endian de NUMFIL
	uint16_t numfil = htons(server_message -> numfil);
    // Mise au format big-endian de NB
	uint16_t nb = htons(server_message -> nb);

    // Copie du message dans un tableau de char
	char data[6];
	memmove(&data[0], &header, 2);
    memmove(&data[2], &numfil, 2);
	memmove(&data[4], &nb, 2);


    // Envoi du tableau
	if (send(fd, data, 6, 0) < 0) {
		perror("write server_message");
        return 1;
	}
	
	return 0;
}

void send_error_message(int fd) {
    server_message_t *error_message = create_server_message(31, 0, 0, 0);
    if (!error_message) {
        perror("create error_message");
        return;
    }

    send_server_message(fd, error_message);
    delete_server_message(error_message);
}

server_message_t *read_server_message(int fd) {
    server_message_t *server_message = calloc(1, sizeof(server_message_t));
    if (!server_message) {
        perror("alloc read server_message");
        goto error;
    }

    uint16_t msg[3];
    if (recv(fd, msg, 6, 0) < 0) {
        perror("recv server message");
        goto error;
    }

    uint16_t header = ntohs(msg[0]);
    server_message -> codereq = header & 0x1F;
    server_message -> id = header >> 5;
    server_message -> numfil = ntohs(msg[1]);
    server_message -> nb = ntohs(msg[2]);

    return server_message;

    error:
        if (server_message)
            delete_server_message(server_message);
        return NULL;
}

void delete_server_message(server_message_t *server_message) {
    if (server_message)
	    free(server_message);
}

post_t *read_post(int sock){
    post_t *posts = calloc(1, sizeof(post_t));
    if (!posts) {
        perror("calloc post");
        if (posts)
            delete_post(posts);
        return NULL;
    }

    char data[23] = {0};

    if(read(sock, data, 23) < 0){
        perror("read");
        return NULL;
    }

    memmove(&(posts->numfil), &data[0], 2);
    posts->numfil = ntohs(posts->numfil);

    memmove(posts->origin, &data[2], 10);
    posts->origin[10] = 0;
    memmove(posts->pseudo, &data[12], 10);
    posts->pseudo[10] = 0;
    memmove(&(posts->datalen), &data[22], 1);
        
    char mes[posts->datalen];
    if(read(sock, mes, posts->datalen) < 0){
        perror("read");
        return NULL;
    }

    posts->data = malloc(posts->datalen + 1);
    memmove(posts->data, &mes[0], posts->datalen);

    return posts;
}

int send_post(int sock, post_t *posts){
    char data[23] = {0};

    uint16_t numfil = htons(posts->numfil);
    memmove(&data[0], &numfil, 2);

    memmove(&data[2], posts->origin, 10);
    memmove(&data[12], posts->pseudo, 10);

    memmove(&data[22], &(posts->datalen), 1);

    if(send(sock, data, 23, 0) < 0){
        perror("send");
        return -1;
    }

    if(send(sock, posts->data, posts->datalen, 0) < 0){
        perror("send");
        return -1;
    }

    return 0;
}

void delete_post(post_t *posts) {
    free(posts->data);
    free(posts);
}

subscribe_t *create_subscribe_message(int codereq, int id, int numfil, int nb, int addrmult) {
    subscribe_t *subscribe_message = calloc(1, sizeof(subscribe_t));
    if (!subscribe_message) {
        perror("alloc subscribe_message");
        goto error;
    }

    // Création du message serveur
    subscribe_message -> server_message = create_server_message(codereq, id, numfil, nb);
    if (!subscribe_message -> server_message) {
        perror("server_message in subscribe_message");
        goto error;
    }

    // Mise au format big-endian de ADDRMULT
    subscribe_message -> addrmult = htonl(addrmult);

    return subscribe_message;

    error:
        if (subscribe_message)
            delete_subscribe_message(subscribe_message);
        return NULL;
}

int send_subscribe_message(int fd, subscribe_t *subscribe_message) {
    if (send_server_message(fd, subscribe_message -> server_message) /* == 1 */) {
        perror("write server_message in subscribe_message");
        return 1;
    }
    
    if (write(fd, &(subscribe_message -> addrmult), 16) < 0) {
        perror("write addrmult in subscribe_message");
        return 1;
    }

    return 0;
}

void delete_subscribe_message(subscribe_t *subscribe_message) {
    if (!subscribe_message)
        return;
    if (subscribe_message -> server_message)
        delete_server_message(subscribe_message -> server_message);
    free(subscribe_message);
}
