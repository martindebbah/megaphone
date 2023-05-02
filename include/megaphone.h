#ifndef MEGAPHONE_H
#define MEGAPHONE_H

// Adresses connexion client et ports
#define ADDR4       "127.0.0.1"
#define ADDR6       "::1"
#define TYPE        4
#define PORT        30000
#define UDP_PORT    30001

// Nombre maximal de threads en simultané
#define NTHREADS 10

// Taille maximale des messages
#define MAX_MESSAGE_SIZE 256

// Librairies utilisées
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

// En-têtes créées
#include "message.h"
#include "server.h"
#include "client.h"
#include "file.h"

#endif
