#include "../include/megaphone.h"

void print(uint16_t t) {
    for (int i = 15; i >= 0; i--) {
        if (t & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

uint16_t create_entete(int codereq, int id) {
    // Initialisation des bits de l'entête à 0
    uint16_t entete = 0;
    // Ajout de ID + décalage de 5 bits vers bit de poids fort
    entete |= (uint16_t) id << 5;
    // Ajout de CODEREQ avec masque des 5 bits de poids faible (0x1F == 0b11111)
    entete |= (uint16_t) (codereq & 0x1F);
    // Mise au format big-endian
    entete = htons(entete);

    return entete;
}

new_client_t *create_new_client(char *pseudo) {
    new_client_t *new_client = calloc(sizeof(new_client_t), 1);
    if (!new_client) {
        perror("alloc new_client");
        goto error;
    }

    // Création de l'entête (CODEREQ = 1, ID = 0)
    new_client -> entete = create_entete(1, 0);

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
    // Copie du billet dans un tableau de char
    char data[12];
    memmove(&data[0], &(new_client -> entete), 2);
    memmove(&data[2], &(new_client -> pseudo), 10);

    // Envoi du tableau
    if (write(fd, data, 12) < 0) {
        perror("write new_client");
        return 1;
    }

    return 0;
}

void delete_new_client(new_client_t *new_client) {
    free(new_client);
}

billet_t *create_billet(int codereq, int id, int numfil, int nb, int datalen, char *data) {
    billet_t *billet = calloc(sizeof(billet_t), 1);
    if (!billet) {
        perror("alloc billet");
        goto error;
    }

    // Création de l'entête
    billet -> entete = create_entete(codereq, id);
    // Mise au format big-endian de numfil
    billet -> numfil = htons((uint16_t) numfil);
    // Mise au format big-endian de nb
    billet -> nb = htons((uint16_t) nb);
    // Ajout de datalen
    billet -> datalen = (uint16_t) datalen;
    // Copie du texte
    billet -> data = calloc(datalen, 1);
    if (!billet -> data) {
        perror("alloc data billet");
        goto error;
    }
    memmove(billet -> data, data, datalen); // Besoin de vérifier bon fonctionnement ?

    return billet;

    error:
        if (billet)
            delete_billet(billet);
        return NULL;
}

int send_billet(int fd, billet_t *billet) {
    // Copie du billet dans un tableau de char
    char data[7 + billet -> datalen];
    memmove(&data[0], &(billet -> entete), 2);
    memmove(&data[2], &(billet -> numfil), 2);
    memmove(&data[4], &(billet -> nb), 2);
    memmove(&data[6], &(billet -> datalen), 1);
    memmove(&data[7], billet -> data, billet -> datalen);

    // Envoi du tableau
    if (write(fd, data, 7 + billet -> datalen) < 0) {
        perror("write billet");
        return 1;
    }

    return 0;
}

void delete_billet(billet_t *billet) {
    if (billet -> data)
        free(billet -> data);
    free(billet);
}
