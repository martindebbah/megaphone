#include "../include/megaphone.h"

#define NTHREADS 10

static users_register_t *users_reg = NULL;
static msg_threads_register_t *msg_threads_reg = NULL;
static pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
static int server_sock = -1;

int main(void) {
	// Initialisation du registre
	create_register();
	if (!users_reg)
		exit(1);
	
	create_msg_threads_register();
	if(!msg_threads_reg)
		exit(1);

	// Mise en place sigaction pour fermeture propre du serveur
	struct sigaction action = {0};
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

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

	pthread_t threads[NTHREADS];
	int *client_sock[NTHREADS] = {NULL};
	while (1) {
		// Accept
		int sock = accept(server_sock, NULL, NULL);
		if (sock == -1) {
			// Fermeture du serveur
			break;
		}

		int i = 0;
		// On cherche un thread libre
		for (; i < NTHREADS; i++) {
			// Si un thread est terminé (que son socket client associé est NULL ou égal à -1)
			if (!client_sock[i] || *client_sock[i] == -1) {
				// Si le thread i n'existait pas
				if (!client_sock[i])
					client_sock[i] = calloc(1, sizeof(int));
				else // Le thread existait, on le join
					pthread_join(threads[i], NULL);

				// Copie du sock client sur le tas
				memmove(client_sock[i], &sock, sizeof(int));

				// Création thread
				if (pthread_create(&threads[i], NULL, serve, client_sock[i]) == -1) {
					perror("Erreur pthread_create");
					close(*client_sock[i]);
					free(client_sock[i]);
				}
				break;
			}
		}

		 // Pas de thread disponible
		if (i == NTHREADS) {
			close(sock);
		}
	}

	// Join tous les threads restants
	for (int i = 0; i < NTHREADS; i++) {
		if (client_sock[i]) {
			pthread_join(threads[i], NULL);
			if (*client_sock[i] != -1)
				close(*client_sock[i]);
			free(client_sock[i]);
		}
	}

	// Libération des ressources
	close (server_sock);
	delete_register();
	delete_msg_threads_register();
	return 0;

	error:
		if (server_sock != -1)
			close(server_sock);
		delete_register();
		delete_msg_threads_register();
		return 1;
}

void print_fil(msg_thread_t *fil){
	if(fil == NULL){
		printf("Fil vide\n");
		return;
	}
	printf("Nb messages : %d\n", fil->nb_msg);
	stack_post_t *tmp = fil->posts;
	while (tmp != NULL){
		printf("Message de %s\n", tmp->post->pseudo);
		printf("Nb de caractères : %d\n", tmp->post->datalen);
		printf("Contenu : %s\n", tmp->post->data);
		tmp = tmp->next;
	}
}

void print_posts(){
	for(int i = 0; i < msg_threads_reg->nb_fils; i++){
		printf("Fils %d :\n", i+1);
		print_fil(msg_threads_reg->msg_threads[i]);
	}
}

void create_register(void) {
	users_reg = calloc(1, sizeof(users_register_t));
	if (!users_reg) {
		perror("Erreur création registre d'utilisateurs");
		return;
	}

	// Initialisation
	users_reg -> users = NULL;
	users_reg -> new_id = 1;
}

int add_user(char *pseudo) {
	// Verrouillage zone critique
	pthread_mutex_lock(&reg_mutex);

	// Réallocation du tableau pour admettre un nouvel utilisateur
	char **new_users = realloc(users_reg -> users, users_reg -> new_id * sizeof(char *));
	if (!new_users) {
		perror("Erreur ajout user");
		pthread_mutex_unlock(&reg_mutex);
		return -1;
	}
	users_reg -> users = new_users;

	// Insertion du pseudo à la position ID - 1
	users_reg -> users[users_reg -> new_id - 1] = calloc(11, 1);
	if (!users_reg -> users[users_reg -> new_id - 1]) {
		perror("Erreur alloc new user");
		pthread_mutex_unlock(&reg_mutex);
		return -1;
	}

	// Copie du pseudo dans le registre
	memmove(users_reg -> users[users_reg -> new_id - 1], pseudo, 10);
	// Incrémentation de l'ID
	int id = users_reg -> new_id;
	users_reg -> new_id = id + 1;
	
	// Déverrouillage zone critique
	pthread_mutex_unlock(&reg_mutex);

	return id;
}

char *get_user(int id) {
	// Critique ?
	// Si l'ID n'est pas valide
	if (!users_reg || id < 1 || id >= users_reg -> new_id)
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

	// Conversion de `data` en `struct new_client_t`
	new_client = read_to_new_client(data);
	if (!new_client) {
		perror("Erreur lecture new_client");
		goto error;
	}

	// Ajout de l'utilisateur dans le registre
	int id = add_user(new_client -> pseudo);
	if (id == -1) {
		perror("Erreur ajout utilisateur");
		goto error;
	}

	// Création du message de réponse
	server_message = create_server_message(1, id, 0, 0);
	if (!server_message) {
		perror("Erreur création server_message");
		goto error;
	}

	// Envoi du message de réponse
	send_server_message(sock, server_message);

	// Libération des ressources
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
	char data[7] = {0};
	if (recv(sock, data, 7, 0) < 0) { // Lire plus
		perror("Erreur recv");
		close(sock);
		return NULL;
	}

	uint16_t header = 0;
	memmove(&header, &data[0], 2);
	// Récupération du CODEREQ
	int codereq = ntohs(header) & 0x1F;
	// Récupération de datalen
	int datalen = 0;
	memmove(&datalen, &data[6], 1);

	char data_complete[7 + datalen];
	memmove(data_complete, data, 7);
	if(recv(sock, &data_complete[7], datalen, 0) < 0) {
		perror("Erreur recv");
		close(sock);
		return NULL;
	}

	//TEST

	//FIN TEST

	// On gère la requête en fonction de son code
	switch (codereq) {
		case 1: // Inscription
			register_new_client(sock, data_complete);
			break;
		case 2: // Poster un billet
			receive_post(sock, data_complete);
			print_posts();
			break;
		case 3: // Liste des n derniers billets
			send_posts(sock, data_complete);
			break;
		case 4: // Abonnement à un fil
			break;
		case 5: // Ajouter un fichier
			break;
		case 6: // Télécharger un fichier
			break;
	}
	// Fermeture socket client
	close(sock);
	// Réécriture du tas pour réutilisabilité du thread
	int closed = -1;
	memmove(arg, &closed, sizeof(int));
	return NULL;
}

void handler(int sig) {
	// Fermeture du socket du serveur pour débloquer l'état `accept()`
	if (server_sock != -1)
		close(server_sock);
}

stack_post_t *create_stack_post(void){
	stack_post_t *stack = calloc(1, sizeof(stack_post_t));
	if (!stack) {
		perror("Erreur création stack");
		return NULL;
	}

	stack -> post = NULL;
	stack -> next = NULL;
	return stack;
}

void delete_stack_post(stack_post_t *stack) {
	if (!stack)
		return;
	if (stack -> post)
		delete_post(stack -> post);
	delete_stack_post(stack -> next);
	free(stack);
}

msg_thread_t *create_msg_thread(char *pseudo){
	msg_thread_t *msg_thread = calloc(1, sizeof(msg_thread_t));
	if (!msg_thread) {
		perror("Erreur création msg_thread");
		return NULL;
	}

	msg_thread -> nb_msg = 0;
	memmove(msg_thread -> pseudo_init, pseudo, 10);
	msg_thread -> posts = create_stack_post();
	return msg_thread;
}

void add_post(msg_thread_t *msg_thread, post_t *post){
	if (!msg_thread || !post)
		return;

	stack_post_t *new_stack = create_stack_post();
	new_stack -> post = post;
	if(msg_thread->posts->post != NULL){
		new_stack->next = msg_thread->posts;
	}
	msg_thread->posts = new_stack;
	msg_thread -> nb_msg++;
}

void delete_msg_thread(msg_thread_t **msg_thread){
	if (!msg_thread)
		return;
	for(int i = 0; i < msg_threads_reg->nb_fils; i++){
		delete_stack_post(msg_threads_reg->msg_threads[i]->posts);
		free(msg_threads_reg->msg_threads[i]);
	}
	free(msg_thread);
}

void create_msg_threads_register(void){
	msg_threads_reg = calloc(1, sizeof(msg_threads_register_t));
	if (!msg_threads_reg) {
		perror("Erreur création msg_threads_register");
		return;
	}

	msg_threads_reg -> msg_threads = NULL;
	msg_threads_reg -> nb_fils = 0;
}

void add_msg_thread(msg_threads_register_t *msg_threads_register, msg_thread_t *msg_thread){
	if (!msg_threads_register || !msg_thread)
		return;

	msg_thread_t **new_msg_threads = realloc(msg_threads_register -> msg_threads, (msg_threads_register -> nb_fils + 2) * sizeof(msg_thread_t *));
	if (!new_msg_threads) {
		perror("Erreur ajout msg_thread");
		return;
	}

	new_msg_threads[msg_threads_register -> nb_fils] = msg_thread;
	new_msg_threads[msg_threads_register -> nb_fils + 1] = NULL;
	msg_threads_register -> msg_threads = new_msg_threads;
	msg_threads_register -> nb_fils++;
}

void delete_msg_threads_register(void){
	if (!msg_threads_reg)
		return;
	if (msg_threads_reg -> msg_threads)
		delete_msg_thread(msg_threads_reg -> msg_threads);
	free(msg_threads_reg);
}

int receive_post(int sock, char *client_data){
	server_message_t *error_msg = create_server_message(31, 0, 0, 0);
	if (!error_msg) {
		goto error;
	}
	uint16_t header = 0;
	memmove(&header, &client_data[0], 2);
	int id = ntohs(header) >> 5;
	int numfil = 0;
	memmove(&numfil, &client_data[2], 2);
	numfil = ntohs(numfil);
	int nb = 0;
	memmove(&nb, &client_data[4], 2);
	nb = ntohs(nb);
	int datalen = 0;
	memmove(&datalen, &client_data[6], 1);
	char *data = calloc(datalen, sizeof(char));
	if (!data) {
		goto error;
	}
	memmove(data, &client_data[7], datalen);

	// si l'utilisateur n'est pas inscrit ou si le fil n'existe pas et n'est pas 0
	if(get_user(id) == NULL || (msg_threads_reg->nb_fils < numfil && numfil != 0)){
		perror("Erreur fil ou utilisateur inexistant");
		goto error;
	}

	// stocker le post du client
	post_t *post = malloc(sizeof(post_t));
	if (!post) {
		perror("Erreur création post");
		goto error;
	}

	post->numfil = numfil;
	memmove(post->pseudo, get_user(id), 10);
	post->pseudo[10] = 0;
	post->datalen = datalen;
	post->data = data;

	if(numfil == 0){
		msg_thread_t *msg_thread = create_msg_thread(get_user(id));
		memmove(post->origin, msg_thread->pseudo_init, 10);
		post->origin[10] = 0;
		add_post(msg_thread, post);
		add_msg_thread(msg_threads_reg, msg_thread);
		post->numfil = msg_threads_reg->nb_fils;
	}
	else {
		memmove(post->origin, msg_threads_reg->msg_threads[numfil-1]->pseudo_init, 10);
		post->origin[10] = 0;
		add_post(msg_threads_reg->msg_threads[numfil-1], post);
	}

	// réponse du serveur à envoyer au client
	server_message_t *server_msg;
	uint16_t numfil_to_send;
	if(numfil == 0){
		numfil_to_send = htons(msg_threads_reg->nb_fils);
	}
	else{
		numfil_to_send = htons(numfil);
	}
	server_msg = create_server_message(2, id, numfil_to_send, 0);
	if(!server_msg){
		goto error;
	}

	if(send_server_message(sock, server_msg) == -1){
		goto error;
	}

	return 0;

	error:
		// message d'erreur à envoyer au client
		if(error_msg && send_server_message(sock, error_msg) == -1){
			perror("Erreur écriture socket");
		}
		return -1;
}

int send_posts(int sock, char *client_data){
	server_message_t *error_msg = create_server_message(31, 0, 0, 0);
	if (!error_msg) {
		goto error;
	}
	uint16_t header = 0;
	memmove(&header, &client_data[0], 2);
	int id = ntohs(header) >> 5;
	int numfil = 0;
	memmove(&numfil, &client_data[2], 2);
	numfil = ntohs(numfil);
	int nb = 0;
	memmove(&nb, &client_data[4], 2);
	nb = ntohs(nb);

	// si l'utilisateur n'est pas inscrit
	if(get_user(id) == NULL){
		perror("Erreur utilisateur inexistant");
		goto error;
	}

	if(nb != 0 && numfil != 0){
		int n = nb;
		if(nb > msg_threads_reg->msg_threads[numfil-1]->nb_msg){
			n = msg_threads_reg->msg_threads[numfil-1]->nb_msg;
		}
		server_message_t *server_msg = create_server_message(3, id, numfil, n);
		if(!server_msg){
			perror("Erreur création server_message");
			goto error;
		}
		if(send_server_message(sock, server_msg) == -1){
			delete_server_message(server_msg);
			goto error;
		}
		delete_server_message(server_msg);

		stack_post_t *tmp = msg_threads_reg->msg_threads[numfil-1]->posts;
		while(tmp != NULL && n > 0){
			if(send_post(sock, tmp->post) == -1){
				goto error;
			}
			tmp = tmp->next;
			n--;
		}
	}

	delete_server_message(error_msg);
	return 0;

	error:
		// message d'erreur à envoyer au client
		if(send_server_message(sock, error_msg) == -1){
			perror("Erreur écriture socket");
		}
		if(error_msg)
			delete_server_message(error_msg);
		return -1;
}