#include "../include/megaphone.h"

// Pour lancer le programme : `./server`

// Registre des utilisateurs et fils de messages
static users_register_t *users_reg = NULL;
static msg_threads_register_t *msg_threads_reg = NULL;

// Mutex du registre des utilisateurs
static pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex du registre des fils
// static pthread_mutex_t *msg_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t recv_post_mutex = PTHREAD_MUTEX_INITIALIZER;

// Socket du serveur
static int server_sock = -1;

// Threads
pthread_t threads[NTHREADS];
thread_args_t *args[NTHREADS] = {NULL};

int main(void) {
	// Initialisation du registre
	create_register();
	if (!users_reg)
		exit(1);
	
	create_msg_threads_register();
	if(!msg_threads_reg)
		goto error;

	if (create_file_dir() != 0)
		goto error;


	// Mise en place sigaction pour fermeture propre du serveur
	struct sigaction action = {0};
	action.sa_handler = handler;
	sigaction(SIGINT, &action, NULL);

	// Socket serveur
	server_sock = socket(PF_INET6, SOCK_STREAM, 0);
	if (server_sock < 0) {
		perror("Erreur socket");
		goto error;
	}

	// Options du socket
	// Réutilisabilité après fermeture
	int opt_reuse = 1;
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt_reuse, sizeof(opt_reuse)) < 0) {
		perror("Erreur option socket reuse");
		goto error;
	}
	// Acceptation des connexions IPv4
	int opt_only6 = 0;
	if (setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt_only6, sizeof(opt_only6)) < 0) {
		perror("Erreur option socket IPv6 only");
		goto error;
	}

	// Structure adresse IPv6
	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(PORT);
	addr.sin6_addr = in6addr_any;

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

	printf("Serveur en attente de connexion\n");

	while (1) {
		// Adresse client pour téléchargement fichier
		struct sockaddr_in6 client_addr;
		socklen_t len = sizeof(client_addr);

		// Accept
		int sock = accept(server_sock, (struct sockaddr *) &(client_addr), &len);
		if (sock == -1) {
			// Fermeture du serveur
			break;
		}

		int i = 0;
		// On cherche un thread libre
		for (; i < NTHREADS; i++) {
			// Si un thread est terminé (si les arguments ne sont pas alloués ou que le socket vaut -1)
			if (!args[i] || args[i] -> sock < 0) {
				// Si le thread i n'existait pas
				if (!args[i]) {
					args[i] = calloc(1, sizeof(thread_args_t));
					if (!args[i]) {
						close(sock);
						goto error;
					}
				}else // Le thread existait, on le join
					pthread_join(threads[i], NULL);

				// Copie du sock et de l'adresse client sur le tas
				memmove(&(args[i] -> sock), &sock, sizeof(int));
				memmove(&(args[i] -> addr), &client_addr, sizeof(client_addr));

				// Création thread
				if (pthread_create(&threads[i], NULL, serve, args[i]) != 0) {
					perror("Erreur pthread_create");
					close(sock);
					free(args[i]);
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
		if (args[i]) {
			pthread_join(threads[i], NULL);
			if (args[i] -> sock >= 0)
				close(args[i] -> sock);
			free(args[i]);
		}
	}

	// Libération des ressources
	close (server_sock);
	delete_register();
	delete_msg_threads_register();
	delete_file_dir();
	return 0;

	error:
		for (int i = 0; i < NTHREADS; i++) {
			if (args[i]) {
				pthread_join(threads[i], NULL);
				if (args[i] -> sock >= 0)
					close(args[i] -> sock);
				free(args[i]);
			}
		}
		if (server_sock != -1)
			close(server_sock);
		delete_register();
		delete_msg_threads_register();
		delete_file_dir();
		return 1;
}

// TODO : Gérer le data[7] cas inscription et autres cas
void *serve(void *arg) {
	thread_args_t *args = (thread_args_t *) arg;
	int sock = args -> sock;

	// Recv
	char entete[2] = {0};
	if(recv(sock, entete, 2, 0) < 0){
		perror("Erreur recv");
		// Fermeture socket client
		close(sock);
		// Réécriture du tas pour réutilisabilité du thread
		int closed = -1;
		memmove(&(args -> sock), &closed, sizeof(int));
		return NULL;
	}

	uint16_t header = 0;
	memmove(&header, &entete[0], 2);
	// Récupération du CODEREQ
	int codereq = ntohs(header) & 0x1F;

	char data_req[10] = {0};
	int datalen = 5; // Dans le cas où on réécrit pas la valeur (inscription) on veut 12 octets
	if(codereq == 1){
		if(recv(sock, data_req, 10, 0) < 0){
			perror("Erreur recv");
			// Fermeture socket client
			close(sock);
			// Réécriture du tas pour réutilisabilité du thread
			int closed = -1;
			memmove(&(args -> sock), &closed, sizeof(int));
			return NULL;
		}
	}else{
		if(recv(sock, data_req, 5, 0) < 0){
			perror("Erreur recv");
			// Fermeture socket client
			close(sock);
			// Réécriture du tas pour réutilisabilité du thread
			int closed = -1;
			memmove(&(args -> sock), &closed, sizeof(int));
			return NULL;
		}

		// Récupération de datalen
		memmove(&datalen, &data_req[4], 1);
	};

	char data_complete[7 + datalen];
	memmove(data_complete, entete, 2);
	if (codereq == 1) {
		memmove(&data_complete[2], data_req, 10);
	}else {
		memmove(&data_complete[2], data_req, 5);
		if(datalen > 0 && recv(sock, &data_complete[7], datalen, 0) < 0) {
			perror("Erreur recv suite data");
			// Fermeture socket client
			close(sock);
			// Réécriture du tas pour réutilisabilité du thread
			int closed = -1;
			memmove(&(args -> sock), &closed, sizeof(int));
			return NULL;
		}
	}

	// Valeur de retour
	int err = 0;

	// On gère la requête en fonction de son code
	switch (codereq) {
		case 1: // Inscription
			err = register_new_client(sock, data_complete);
			break;
		case 2: // Poster un billet
			err = receive_post(sock, data_complete);
			// print_posts();
			break;
		case 3: // Liste des n derniers billets
			err = send_posts(sock, data_complete);
			break;
		case 4: // Abonnement à un fil
			break;
		case 5: // Ajouter un fichier
			err = add_file(sock, data_complete);
			break;
		case 6: // Télécharger un fichier
			err = download_file(sock, data_complete, args -> addr);
			break;
		default:
			err = 1;
	}

	// Gestion des erreurs
	if (err)
		send_error_message(sock);
	
	// Fermeture socket client
	close(sock);
	// Réécriture du tas pour réutilisabilité du thread
	int closed = -1;
	memmove(&(args -> sock), &closed, sizeof(int));
	return NULL;
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

	char *user = get_user(server_message -> id);
	remove_hash(user);
	printf("[%d] Enregistrement de %s avec l'id %d\n",
			server_message -> codereq, user, server_message -> id);
	free(user);

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

int add_file(int sock, char *data) {
	client_message_t *client_message = NULL;
	server_message_t *server_message = NULL;
	file_t *file = NULL;
	int udp_sock = -1;

	// Conversion de `data` en `struct client_message`
	client_message = read_to_client_message(data);
	if (!client_message) {
		perror("Erreur add_file client_message");
		goto error;
	}

	if (is_client_valid(client_message) != 0)
		goto error;

	// Création du message de réponse du serveur
	server_message = create_server_message(client_message -> codereq, client_message -> id,
											client_message -> numfil, UDP_PORT);
	if (!client_message) {
		perror("Erreur add_file client_message");
		goto error;
	}

	// Initialisation du socket UDP en IPv6
	udp_sock = socket(PF_INET6, SOCK_DGRAM, 0);
	if (udp_sock < 0) {
		perror("Erreur init udp_sock");
		goto error;
	}

	// Structure adresse IPv6
	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_any;
	addr.sin6_port = htons(UDP_PORT);

	// Bind
	if (bind(udp_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Erreur bind udp_sock");
		goto error;
	}

	file = create_file(client_message -> data, client_message -> datalen);
	if (!file) {
		perror("Erreur file");
		goto error;
	}

	// Envoi du message serveur
	if (send_server_message(sock, server_message) != 0) {
		perror("Erreur add_file envoi serveur message");
		goto error;
	}

	// Lecture du fichier sur le socket UDP
	if (recv_file(udp_sock, file) != 0) {
		perror("lecture file udp");
		goto error;
	}

	// Mutex
	if (add_file_to_thread(client_message -> numfil, file, client_message -> id) == -1) {
		goto error;
	}
	// Fin mutex

	// Dernier message pour valider l'ajout
	int numfil = client_message -> numfil == 0 ? msg_threads_reg -> nb_fils : client_message -> numfil;
	delete_server_message(server_message);
	server_message = create_server_message(client_message -> codereq, client_message -> id,
											numfil, client_message -> nb);
	if (!server_message) {
		perror("dernier server_message add_file");
		goto error;
	}

	if (send_server_message(sock, server_message) != 0) {
		perror("Erreur add_file envoi serveur message");
		goto error;
	}

	char filename[MAX_MESSAGE_SIZE] = {0};
	memmove(filename, file -> name, file -> name_len);

	printf("[%d] Ajout de %s par l'utilisateur %d sur le fil %d\n",
			client_message -> codereq, filename, client_message -> id, numfil);

	delete_client_message(client_message);
	delete_server_message(server_message);
	delete_file(file);
	close(udp_sock);

	return 0;

	error:
		if (client_message)
			delete_client_message(client_message);
		if (server_message)
			delete_server_message(server_message);
		if (file)
			delete_file(file);
		if (udp_sock != -1)
			close(udp_sock);
		return 1;
}

int download_file(int sock, char *data, struct sockaddr_in6 addr) {
	client_message_t *client_message = NULL;
	server_message_t *server_message = NULL;
	int file = -1;
	int udp_sock = -1;
	char *filename = NULL;

	// Message client
	client_message = read_to_client_message(data);
	if (!client_message)
		goto error;

	if (is_client_valid(client_message) != 0)
		goto error;

	char path[100] = {0};
	filename = calloc(client_message -> datalen + 1, 1);
	memmove(filename, client_message -> data, client_message -> datalen);
	snprintf(path, 100, "fichiers/fil_%d/%s", client_message -> numfil, filename);
	file = open(path, O_RDONLY);
	if (file < 0) {
		perror("open file download_file");
		goto error;
	}

	// Socket UDP
	udp_sock = socket(PF_INET6, SOCK_DGRAM, 0);
	if (udp_sock < 0) {
		perror("socket UDP download_file");
		goto error;
	}

	// Changement du port sur l'adresse client
	addr.sin6_port = htons(client_message -> nb);

	// Réponse du serveur
	server_message = create_server_message(client_message -> codereq, client_message -> id,
											client_message -> numfil, client_message -> nb);
	if (!server_message) {
		perror("server_message download_file");
		goto error;
	}

	if (send_server_message(sock, server_message) != 0) {
		perror("send download_file");
		goto error;
	}

	// Envoi du fichier
	if (send_file(udp_sock, addr, file, client_message -> id) != 0) {
		perror("send_file download_file");
		// Envoi d'erreur ?
	}

	printf("[%d] Envoi de %s à l'utilisateur %d depuis le fil %d\n",
			client_message -> codereq, filename, client_message -> id, client_message -> numfil);

	delete_client_message(client_message);
	delete_server_message(server_message);
	free(filename);
	close(file);
	close(udp_sock);

	return 0;

	error:
		if (client_message)
			delete_client_message(client_message);
		if (server_message)
			delete_server_message(server_message);
		if (file >= 0)
			close(file);
		if (udp_sock >= 0)
			close(udp_sock);
		if (filename)
			free(filename);
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

void print_posts(void){
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
	msg_thread -> posts = NULL;
	return msg_thread;
}

int add_post(msg_thread_t *msg_thread, post_t *post){
	if (!msg_thread || !post)
		return -1;

	stack_post_t *new_stack = create_stack_post();
	if(!new_stack)
		return -1;
	new_stack -> post = post;
	new_stack->next = msg_thread->posts;
	msg_thread->posts = new_stack;
	msg_thread -> nb_msg++;
	return 0;
}

void delete_msg_thread(msg_thread_t *msg_thread){
	if (!msg_thread)
		return;
	delete_stack_post(msg_thread->posts);
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

int add_msg_thread(msg_threads_register_t *msg_threads_register, msg_thread_t *msg_thread){
	if (!msg_threads_register || !msg_thread)
		return -1;

	msg_thread_t **new_msg_threads = realloc(msg_threads_register -> msg_threads, (msg_threads_register -> nb_fils + 2) * sizeof(msg_thread_t *));
	if (!new_msg_threads) {
		perror("Erreur ajout msg_thread");
		return -1;
	}

	new_msg_threads[msg_threads_register -> nb_fils] = msg_thread;
	new_msg_threads[msg_threads_register -> nb_fils + 1] = NULL;
	msg_threads_register -> msg_threads = new_msg_threads;
	msg_threads_register -> nb_fils++;
	return 0;
}

void delete_msg_threads_register(void){
	if (!msg_threads_reg)
		return;
	if (msg_threads_reg -> msg_threads) {
		for (int i = 0; i < msg_threads_reg -> nb_fils; i++){
			delete_msg_thread(msg_threads_reg -> msg_threads[i]);
		}
		free(msg_threads_reg -> msg_threads);
	}
	free(msg_threads_reg);
}

int receive_post(int sock, char *client_data){
	client_message_t *client_message = NULL;
	server_message_t *server_msg = NULL;
	post_t *post = NULL;
	char *user = NULL;
	
	client_message = read_to_client_message(client_data);
	if (!client_message) {
		perror("client_message receive_post");
		goto error;
	}

	// si l'user ou le fil n'existe pas
	if (is_client_valid(client_message) != 0)
		goto error;

	user = get_user(client_message -> id);

	// stocker le post du client
	post = malloc(sizeof(post_t));
	if (!post) {
		perror("Erreur création post");
		goto error;
	}

	post->numfil = client_message -> numfil;
	memmove(post->pseudo, user, 10);
	post->pseudo[10] = 0;
	post->datalen = client_message -> datalen;
	post -> data = calloc(post -> datalen, 1);
	memmove(post -> data, client_message -> data, post -> datalen);

	// mutex
	pthread_mutex_lock(&recv_post_mutex);
	if(client_message -> numfil == 0){
		msg_thread_t *msg_thread = create_msg_thread(user);
		memmove(post->origin, msg_thread->pseudo_init, 10);
		post->origin[10] = 0;
		if(add_post(msg_thread, post) != 0){
			pthread_mutex_unlock(&recv_post_mutex);
			goto error;
		}
		if(add_msg_thread(msg_threads_reg, msg_thread) != 0){
			pthread_mutex_unlock(&recv_post_mutex);
			goto error;
		}
		post->numfil = msg_threads_reg->nb_fils;
	}
	else {
		memmove(post->origin, msg_threads_reg->msg_threads[client_message -> numfil-1]->pseudo_init, 10);
		post->origin[10] = 0;
		if(add_post(msg_threads_reg->msg_threads[client_message -> numfil-1], post) != 0){
			pthread_mutex_unlock(&recv_post_mutex);
			goto error;
		}
	}
	// fin mutex
	pthread_mutex_unlock(&recv_post_mutex);

	// réponse du serveur à envoyer au client
	int numfil = client_message -> numfil == 0 ? msg_threads_reg -> nb_fils : client_message -> numfil;

	server_msg = create_server_message(2, client_message -> id, numfil, 0);
	if(!server_msg) {
		perror("server_message receive_post");
		goto error;
	}

	if(send_server_message(sock, server_msg) == -1) {
		perror("send server_message receive_post");
		goto error;
	}

	printf("[%d] Ajout d'un post de l'utilisateur %d au fil %d\n",
			server_msg -> codereq, server_msg -> id, server_msg -> numfil);

	delete_client_message(client_message);
	delete_server_message(server_msg);
	free(user);
	return 0;

	error:
		if (client_message)
			delete_client_message(client_message);
		if (post)
			delete_post(post);
		if (server_msg)
			delete_server_message(server_msg);
		if (user)
			free(user);
		return -1;
}

int send_posts(int sock, char *client_data){
	server_message_t *server_msg = NULL;
	client_message_t *client_message = read_to_client_message(client_data);
	if (!client_message) {
		perror("client_message send_posts");
		goto error;
	}

	// si l'user ou le fil n'existe pas
	if (is_client_valid(client_message) != 0)
		goto error;

	int n = client_message -> nb;
	int f = client_message -> numfil;
	pthread_mutex_lock(&recv_post_mutex);
	if(client_message -> numfil == 0){
		f = msg_threads_reg->nb_fils;
		if(n == 0){
			for(int i = 0; i < msg_threads_reg->nb_fils; i++){
				if(msg_threads_reg->msg_threads[i]->nb_msg < client_message -> nb || client_message -> nb == 0){
					n += msg_threads_reg->msg_threads[i]->nb_msg;
				}
				else{
					n += client_message -> nb;
				}
			}
		}
		else{
			n = client_message -> nb * msg_threads_reg->nb_fils;
		}
	}
	else {
		if(client_message -> nb > msg_threads_reg->msg_threads[client_message -> numfil-1]->nb_msg || client_message -> nb == 0){
			n = msg_threads_reg->msg_threads[client_message -> numfil-1]->nb_msg;
		}
	}
	pthread_mutex_unlock(&recv_post_mutex);

	server_msg = create_server_message(3, client_message -> id, f, n);
	if(!server_msg){
		perror("Erreur création server_message");
		goto error;
	}
	if(send_server_message(sock, server_msg) == -1){
		goto error;
	}
	// mutex
	pthread_mutex_lock(&recv_post_mutex);
	if(msg_threads_reg->nb_fils > 0){
		if(client_message -> numfil == 0){
			for(int i = 0; i < msg_threads_reg->nb_fils; i++){
				stack_post_t *tmp = msg_threads_reg->msg_threads[i]->posts;
				if(client_message -> nb > msg_threads_reg->msg_threads[i]->nb_msg || client_message -> nb == 0){
					n = msg_threads_reg->msg_threads[i]->nb_msg;
				}
				else{
					n = client_message -> nb;
				}
				while(tmp != NULL && n > 0){
					if(send_post(sock, tmp->post) == -1){
						pthread_mutex_unlock(&recv_post_mutex);
						goto error;
					}
					tmp = tmp->next;
					n--;
				}
			}
		}
		else {
			stack_post_t *tmp = msg_threads_reg->msg_threads[client_message -> numfil-1]->posts;
			while(tmp != NULL && n > 0){
				if(send_post(sock, tmp->post) == -1){
					pthread_mutex_unlock(&recv_post_mutex);
					goto error;
				}
				tmp = tmp->next;
				n--;
			}
		}
	}
	// fin mutex
	pthread_mutex_unlock(&recv_post_mutex);

	printf("[%d] Envoi de %d messages à l'utilisateur %d\n",
			server_msg -> codereq, server_msg -> nb, server_msg -> id);

	delete_client_message(client_message);
	delete_server_message(server_msg);

	return 0;

	error:
		if (client_message)
			delete_client_message(client_message);
		if (server_msg)
			delete_server_message(server_msg);
		// message d'erreur à envoyer au client
		return -1;
}

int add_file_to_thread(uint16_t numfil, file_t *file, int id) {
	char *user = NULL;
	msg_thread_t *msg_thread = NULL;
	post_t *post = NULL;
	char *file_size = NULL;

	user = get_user(id);
	if (!user) {
		perror("get_user add_file");
		goto error;
	}
	if (numfil == 0) { // Création d'un nouveau fil
		msg_thread = create_msg_thread(user);
		numfil = (uint16_t) (msg_threads_reg -> nb_fils + 1);
	}

	char path[20] = {0};
	snprintf(path, sizeof(path), "fichiers/fil_%d", numfil);

	DIR *dir = opendir(path);
	if (!dir) {
		if (mkdir(path, 0755) != 0) {
			perror("Création répertoire");
			goto error;
		}
	}else {
		closedir(dir);
	}

	char file_path[100] = {0};
	snprintf(file_path, sizeof(file_path), "%s/%s", path, file -> name);

	if (write_file(file_path, file) != 0) {
		perror("Ecriture fichier");
		goto error;
	}

	add_msg_thread(msg_threads_reg, msg_thread); // Vérifier

	post = calloc(1, sizeof(post_t));
	if (!post) {
		perror("alloc post add_file");
		goto error;
	}
	post -> numfil = numfil;
	memmove(post -> origin, msg_threads_reg -> msg_threads[numfil - 1] -> pseudo_init, 10);
	post -> origin[10] = 0;
	memmove(post -> pseudo, user, 10);
	post -> pseudo[10] = 0;

	file_size = str_of_size_file(file);
	post -> datalen = file -> name_len + strlen(file_size) + 2;
	post -> data = calloc(post -> datalen, 1);
	memmove(post -> data, file -> name, file -> name_len);
	post -> data[file -> name_len] = ' ';
	memmove(&(post -> data[file -> name_len + 1]), file_size, strlen(file_size));
	post -> data[post -> datalen - 1] = 'o';

	add_post(msg_threads_reg -> msg_threads[numfil - 1], post); // Vérifier

	free(user);
	free(file_size);

	return 0;

	error:
		if (user)
			free(user);
		if (msg_thread)
			delete_msg_thread(msg_thread);
		if (post)
			delete_post(post);
		if (file_size)
			free(file_size);
		return 1;
}

int create_file_dir(void) {
	if (mkdir("fichiers", 0755) != 0)
		return 1;
	return 0;
}

void supp_rep(char *path) {
	DIR *dir = opendir(path);
	struct dirent *entry;
	if (!dir)
		return;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry -> d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0)
			continue;
		
		char entry_path[1024] = {0};
		snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry -> d_name);
		if (entry -> d_type == DT_DIR) {
			supp_rep(entry_path);
		}else {
			unlink(entry_path);
		}
	}

	rmdir(path);
	closedir(dir);
}

void delete_file_dir(void) {
	supp_rep("fichiers");
}

int is_client_valid(client_message_t *client_message) {
	// Numfil inexistant
	if (client_message -> numfil < 0 || client_message -> numfil > msg_threads_reg -> nb_fils) {
		printf("Fil inexistant\n");
		return 1;
	}

	// ID inexistant
	if (client_message -> id < 1 || client_message -> id >= users_reg -> new_id) {
		printf("Id utilisateur inexistant\n");
		return 1;
	}

	return 0;
}
