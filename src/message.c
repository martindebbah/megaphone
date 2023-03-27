#include "../include/megaphone.h"

// Affiche deux octets (de gauche à droite: poids fort -> poids faible)
void print(uint16_t t) {
    for (int i = 15; i >= 0; i--) {
        if (t & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

// Affiche un octet (de gauche à droite: poids fort -> poids faible)
void print_8(uint8_t t) { // A retirer
    for (int i = 7; i >= 0; i--) {
        if (t & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

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
    new_client -> header = create_header(1, 0);

    // Remplissage du pseudo
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
    // Copie du message dans un tableau de char
    char data[12];
    memmove(&data[0], &(new_client -> header), 2);
    memmove(&data[2], &(new_client -> pseudo), 10);

    // Envoi du tableau
    if (write(fd, data, 12) < 0) {
        perror("write new_client");
        return 1;
    }

    return 0;
}

void delete_new_client(new_client_t *new_client) {
    if (new_client)
        free(new_client);
}

client_message_t *create_client_client_message(int codereq, int id, int numfil, int nb, int datalen, char *data) {
    client_message_t *client_message = calloc(1, sizeof(client_message_t));
    if (!client_message) {
        perror("alloc client_message");
        goto error;
    }

    // Création de l'en-tête
    client_message -> header = create_header(codereq, id);
    // Mise au format big-endian de numfil
    client_message -> numfil = htons((uint16_t) numfil);
    // Mise au format big-endian de nb
    client_message -> nb = htons((uint16_t) nb);
    // Ajout de datalen
    client_message -> datalen = (uint16_t) datalen;
    // Copie du texte
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
    // Copie du client_message dans un tableau de char
    char data[7 + client_message -> datalen];
    memmove(&data[0], &(client_message -> header), 2);
    memmove(&data[2], &(client_message -> numfil), 2);
    memmove(&data[4], &(client_message -> nb), 2);
    memmove(&data[6], &(client_message -> datalen), 1);
    memmove(&data[7], client_message -> data, client_message -> datalen);

    // Envoi du tableau
    if (write(fd, data, 7 + client_message -> datalen) < 0) {
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

    // Création de l'en-tête
	server_message -> header = create_header(codereq, id);
    // Mise au format big-endian de numfil
	server_message -> numfil = htons((uint16_t) numfil);
    // Mise au format big-endian de nb
	server_message -> nb = htons((uint16_t) nb);

	return (server_message);
}

int send_server_message(int fd, server_message_t *server_message) {
    // Copie du message dans un tableau de char
	char data[6];
	memmove(&data[0], &(server_message -> header), 2);
    memmove(&data[2], &(server_message -> numfil), 2);
	memmove(&data[2], &(server_message -> nb), 2);


    // Envoi du tableau
	if (write(fd, data, 6) < 0) {
		perror("write server_message");
        return 1;
	}
	
	return 0;
}

void delete_server_message(server_message_t *server_message) {
    if (server_message)
	    free(server_message);
}
