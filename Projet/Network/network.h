#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>

#include "client.h"
#include "server.h"
#include "../Hash/sha256.h"


#define ERROR -1
#define ENDED 0
#define CONNECTED 1
#define NOSTARTED 2
#define CONNECTING 3
#define NOTUSED 255

struct clientInfo
{
    size_t ID;
    struct sockaddr *IP;
    int fd;
    int fdin;
    int fdout;
    int fdinThread;
    int fdoutThread;
    int status;
};

struct listClientInfo
{
    size_t size;
    struct clientInfo *list;
};

int network(int fdin, int fdout);
struct clientInfo * initClient(struct listClientInfo *clients);
void *transmit(void *arg);
void freeClient(struct clientInfo client, struct listClientInfo *clients);
void removeClient(struct clientInfo client, struct listClientInfo *clients);



#endif