# include "../include/megaphone.h"

// Pour lancer le programme on lance `client.out`
// Et on fait `netcat -l 7777`

// On travaille sur lulu
#define ADDR "192.168.70.236"
// Port défini par le sujet
#define PORT 7777

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
    

    close(sock);
    // `echo $?` pour valeur de retour
    exit(0);
}