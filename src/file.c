#include "../include/megaphone.h"

file_t *create_file(char *name, int name_len) {
    file_t *file = calloc(1, sizeof(file_t));
    if (!file) {
        perror("Erreur calloc file");
        goto error;
    }

    file -> name = calloc(name_len + 1, 1);
    if (!file -> name) {
        perror("Erreur calloc file name");
        goto error;
    }

    memmove(file -> name, name, name_len);
    file -> name_len = name_len;
    return file;

    error:
        if (file)
            delete_file(file);
        return NULL;
}

int add_data(file_t *file, udp_t *data) {
    if (file -> data[data -> numblock])
        return 1;
    file -> data[data -> numblock - 1] = data;
    return 0;
}

int is_complete(file_t *file) {
    int complete = 0;
    for (int i = 0; i < NPAQUETS; i++) {
        if (!file -> data[i])
            break;
        if (strnlen(file -> data[i] -> paquet, 512) < 512)
            complete = 1;
    }
    return complete;
}

int send_file(int sock, struct sockaddr_in6 addr, int file, int id) {
    int reading = 1;
    int numblock = 1;

    while (reading) {
        char buffer[UDP_PACKET_SIZE] = {0};
        int nread = read(file, buffer, UDP_PACKET_SIZE);
        if (nread < 0) {
            perror("read send_file");
            return 1;
        }

        if (nread == 0) { // Fin du fichier
            reading = 0;
        }else {
            udp_t *udp = create_udp(id, numblock, buffer);
            if (!udp) {
                perror("create_udp send_file");
                return 1;
            }

            int sent = send_udp(sock, addr, udp);
            delete_udp(udp);
            if (sent != 0) {
                perror("send_udp send_file");
                return 1;
            }
            numblock++;
        }
    }

    return 0;
}

char *str_of_size_file(file_t *file) {
    int i = 0;
    int size = 0;
    while (1) {
        if (!file -> data[i])
            break;
        size += strnlen(file -> data[i] -> paquet, 512);
        i++;
    }
    char *str = calloc(8, 1);
    snprintf(str, 8, "%d", size);
    return str;
}

int recv_file(int sock, file_t *file) {
    struct timeval udp_timeout;
	udp_timeout.tv_sec = 5;
	udp_timeout.tv_usec = 0;

    while (is_complete(file) == 0) {
        char buffer[UDP_PACKET_SIZE + 4] = {0};

        // Pour ne pas bloquer si un paquet est perdu
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sock, &set);
		int sel = select(sock + 1, &set, NULL, NULL, &udp_timeout);
		if (sel <= 0) {
            if (errno == 0) { // Timeout
                printf("Les données ne sont pas complètes\nTéléchargement annulé\n\n");
                return 0;
            }
            perror("select add_file");
			return 1;
		}

        int n = recvfrom(sock, buffer, 516, 0, NULL, NULL);
		if (n < 0) {
			perror("Erreur recvfrom udp");
			return 1;
		}

        udp_t *udp = read_to_udp(buffer);
		if (!udp) {
			perror("Erreur udp");
			delete_udp(udp);
			return 1;
		}

        if (add_data(file, udp) == 1) {
			perror("Erreur ajout udp");
			delete_udp(udp);
			return 1;
		}
    }

    return 0;
}

int write_file(char *path, file_t *file) {
    int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0777);
	if (fd < 0)
		return 1;

    int i = 0;
    while (1) {
        if (!file -> data[i])
            break;

        // Si c'est le dernier bloc, on ne copie pas les '\0'
        int size = strlen(file -> data[i] -> paquet) < UDP_PACKET_SIZE ? strlen(file -> data[i] -> paquet) : UDP_PACKET_SIZE;

        int nwritten = write(fd, file -> data[i] -> paquet, size);
        if (nwritten < 0) {
            perror("write_file");
            unlink(path);
            close(fd);
            return 1;
        }
        i++;
    }

    close(fd);
    return 0;
}

void delete_file(file_t *file) {
    if (!file)
        return;
    if (file -> name)
        free(file -> name);
    for (int i = 0; i < NPAQUETS; i++)
        if (file -> data[i])
            delete_udp(file -> data[i]);
    free(file);
}
