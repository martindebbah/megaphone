#ifndef MEGAPHONE_H
#define MEGAPHONE_H

// Adresses connexion client et ports
#define PORT_STR    "30000"
#define PORT        30000
#define UDP_PORT    29999

// Nombre maximal de threads en simultané
#define NTHREADS 10

// Taille maximale des messages
#define MAX_MESSAGE_SIZE    256
#define UDP_PACKET_SIZE     512

#define NAME_INTERFACE "eth0"

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
#include <sys/stat.h>
#include <dirent.h>
#include <net/if.h>

// En-têtes créées
#include "message.h"
#include "file.h"
#include "server.h"
#include "client.h"
#include "utils.h"

#endif
