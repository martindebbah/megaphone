#include "../include/megaphone.h"

static users_register_t *users_reg;

int main(void) {
	// Initialisation des variables
	create_register();
	int server_sock = -1;

	// Socket
	server_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		perror("Erreur socket");
		goto error;
	}
	int opt = 1;
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	// IPv4 pour l'instant
	addr.sin_family = AF_INET;
	addr.sin_port = htons(30000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind
	int b = bind(server_sock, (struct sockaddr *) &addr, sizeof(addr));
	if (b == -1) {
		perror("Erreur bind socket");
		goto error;
	}
	
	// Listen
	int l = listen(server_sock, 0);
	if (l == -1) {
		perror("Erreur listen");
		goto error;
	}

	while (1) {
		// Accept
		int *client_sock = calloc(1, sizeof(int));
		*client_sock = accept(server_sock, NULL, NULL);
		if (*client_sock == -1) {
			perror("Erreur accept");
			goto error;
			// Peut-être juste skip et attendre le prochain accept
		}

		// Création thread
		pthread_t thread;
		if (pthread_create(&thread, NULL, serve, client_sock) == -1) {
			perror("Erreur pthread_create");
			continue;
		}

		// Fermeture des processus fils terminés
	}

	// Close
	close (server_sock);
	delete_register();
	return 0;

	error:
		if (server_sock != -1)
			close(server_sock);
		delete_register();
		return 1;
}

void create_register(void) {
	users_reg = calloc(1, sizeof(users_register_t));
	if (!users_reg) {
		perror("Erreur création registre d'utilisateurs");
		return;
	}
	users_reg -> users = NULL;
	users_reg -> new_id = 1;
}

int add_user(char *pseudo) {
	// Verrou

	// Réallocation du tableau pour admettre un nouvel utilisateur
	char **new_users = realloc(users_reg -> users, users_reg -> new_id * sizeof(char *));
	if (new_users == NULL) {
		perror("Erreur ajout user");
		return -1;
	}
	users_reg -> users = new_users;
	// Insertion du pseudo à la position ID - 1
	users_reg -> users[users_reg -> new_id - 1] = calloc(11, 1);
	if (!users_reg -> users[users_reg -> new_id - 1]) {
		perror("Erreur alloc new user");
		return -1;
	}
	memmove(users_reg -> users[users_reg -> new_id - 1], pseudo, 10);
	// Incrémentation de l'ID
	int id = users_reg -> new_id;
	users_reg -> new_id = id + 1;
	
	// Fin verrou

	return id;
}

char *get_user(int id) {
	if (id < 1 || id >= users_reg -> new_id)
		return NULL;
	char *pseudo = calloc(11, 1);
	if (!pseudo) {
		perror("Erreur alloc get_user");
		return NULL;
	}
	// ID à la position ID - 1
	memmove(pseudo, users_reg -> users[id - 1], 10);
	return pseudo;
}

void delete_register(void) {
	if (!users_reg)
		return;
	
	if (users_reg -> users) {
		for (int i = 0; i < users_reg -> new_id - 1; i++)
			free(users_reg -> users[i]);
		free(users_reg -> users);
	}
	free(users_reg);
}

int register_new_client(int sock, char *data) {
	new_client_t *new_client = NULL;
	server_message_t *server_message = NULL;
	new_client = read_to_new_client(data);
	if (!new_client) {
		perror("Erreur lecture new_client");
		goto error;
	}
	int id = add_user(new_client -> pseudo);
	if (id == -1) {
		perror("Erreur ajout utilisateur");
		goto error;
	}
	server_message = create_server_message(1, id, 0, 0);
	if (!server_message) {
		perror("Erreur création server_message");
		goto error;
	}

	send_server_message(sock, server_message);

	delete_new_client(new_client);
	delete_server_message(server_message);
	return 0;

	error:
		if (new_client)
			delete_new_client(new_client);
		if (server_message)
			delete_server_message(server_message);
		return 1;
}

void *serve(void *arg) {
	int sock = *((int *) arg);
	// Recv
	char data[12] = {0};
	if (recv(sock, data, 12, 0) < 0) { // Lire plus
		perror("Erreur recv");
		close(sock);
		return NULL;
	}

	uint16_t header = 0;
	memmove(&header, &data[0], 2);
	// Récupération du CODEREQ
	int codereq = ntohs(header) & 0x1F;

	// On gère la requête en fonction de son code
	switch (codereq) {
		case 1: // Inscription
			register_new_client(sock, data);
			break;
		case 2: // Poster un billet
			break;
		case 3: // Liste des n derniers billets
			break;
		case 4: // Abonnement à un fil
			break;
		case 5: // Ajouter un fichier
			break;
		case 6: // Télécharger un fichier
			break;
	}
	close(sock);
	return NULL;
}
