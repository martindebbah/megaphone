#include "../include/megaphone.h"

entete_t *create_entete(int codereq, int id) {
    entete_t *entete = malloc(sizeof(entete));
    if (!entete) {
        perror("malloc entete");
        goto error;
    }

    // Initialisation des bits de l'entête à 0
    entete -> entete = 0;
    // Ajout de ID + décalage de 5 bits vers bit de poids fort
    entete -> entete |= (uint16_t) (id << 5);
    // Ajout de CODEREQ avec masque des 5 bits de poids faible (0x1F == 0b11111)
    entete -> entete |= (uint16_t) (codereq & 0x1F);
    // Transformation en big-endian
    entete -> entete = htons(entete -> entete);

    return entete;

    error:
        if (entete)
            delete_entete(entete);
        return NULL;
}

void delete_entete(entete_t *entete) {
    free(entete);
}

new_client_t *create_new_client(char *pseudo) {
    new_client_t *new_client = malloc(sizeof(new_client_t));
    if (!new_client) {
        perror("malloc new_client");
        goto error;
    }

    // Création de l'entête (CODEREQ = 1, ID = 0)
    new_client -> entete = create_entete(1, 0);
    if (!new_client -> entete)
        goto error;

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

void delete_new_client(new_client_t *new_client) {
    if (new_client -> entete)
        delete_entete(new_client -> entete);
    free(new_client);
}
