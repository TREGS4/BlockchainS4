#include "network.h"

struct descripAndClients
{
    struct listClientInfo *clients;
    int fdin;
    int fdout;
};

void *test(void *arg)
{
    while (1)
    {

        struct listClientInfo *clients = arg;
        for (size_t i = 0; i < clients->size; i++)
        {
            if (clients->list[i].status == NOTUSED)
                printf("%ld is not used\n", i);
            else if (clients->list[i].status == CONNECTED)
            {
                printf("%ld is connected\n", i);
                //printf("%ld IP is %s\n", i, clients->list[i].IP->sa_data);
            }
            else
                printf("%ld is autre chose\n", i);
        }
        printf("\n\n");
        sleep(2);
    }
    return NULL;
}

int network(int fdin, int fdout)
{
    struct listClientInfo clients;
    clients.size = 1;
    clients.list = malloc(clients.size * sizeof(struct clientInfo));
    pthread_t ServerThread;
    pthread_t MaintenerThread;
    pthread_t transmitThread;
    struct descripAndClients fdCli;
    fdCli.clients = &clients;
    fdCli.fdin = fdin;
    fdCli.fdout = fdout;
    pthread_mutex_init(&clients.lockList, NULL);
    pthread_mutex_init(&clients.lockRead, NULL);
    pthread_mutex_init(&clients.lockWrite, NULL);

    for (size_t i = 0; i < clients.size; i++)
        clients.list[i].status = NOTUSED;
    

    //connect()

    pthread_create(&ServerThread, NULL, server, (void *)&clients);
    pthread_create(&MaintenerThread, NULL, connectionMaintener, (void *)&clients);
    //pthread_create(&transmitThread, NULL, test, (void *)&clients);

    pthread_join(ServerThread, NULL);
    pthread_join(MaintenerThread, NULL);
    //pthread_join(transmitThread, NULL);

    free(clients.list);

    return EXIT_SUCCESS;
}

int compareString(char str1[14], char *str2)
{
    int res = 1;
    size_t pos = 0;

    while (res && str1[pos] != '\0' && str2[pos] != '\0')
    {
        if (str1[pos] != str2[pos])
            res = 0;
        pos++;
    }

    if (str1[pos] != str2[pos])
        res = 0;

    return res;
}

size_t findClient(struct listClientInfo *clients, char *buff)
{
    size_t res = 0;
    int find = 0;
    size_t i = 0;

    while (i < clients->size && !find)
    {
        if (clients->list[i].status == CONNECTED && compareString(clients->list[i].IP.sa_data, buff))
            find = 1;
        else
            i++;
    }

    if (!find)
        res = clients->size * 2;

    return res;
}

void *transmit(void *arg)
{
    struct descripAndClients *fds = arg;
    char buff[4];
    char buff2[512];
    int r = 0;
    int pos2 = 0;
    int correct = 0;
    int sizeIPV4 = 4;
    int reste = sizeIPV4 + 1;
    size_t pos;
    while (1)
    {
        correct = 0;
        r = 0;
        while (correct == 0)
        {
            r = read(STDIN_FILENO, buff + pos2, reste);
            reste -= r;
            pos2 += r;
            if (reste == 0 && buff[pos2] != '\n')
                correct = 1;
            else
            {
                pos2 = 0;
                reste = sizeIPV4 + 1;
            }
        }

        if (r != 0)
        {
            pos = findClient(fds->clients, buff);
            if (pos != fds->clients->size * 2)
            {
                int stop = 0;
                while (!stop)
                {
                    r = read(fds->fdin, buff2, 512);
                    write(fds->clients->list[pos].fdinThread, buff, r);
                    if (buff[r - 1] == '\n')
                        stop = 1;
                }
            }
            else
                printf("The server is not in the list of connected server\n");
        }
        else
            printf("Error while receiving the IP of the server\n");
    }
}

/*
Double the size of the list
*/

void extendList(struct listClientInfo *ptr)
{
    pthread_mutex_lock(&ptr->lockList);
    ptr->size *= 2;

    ptr->list = realloc(ptr->list, ptr->size * sizeof(*ptr->list));
    if (ptr->list == NULL)
        err(EXIT_FAILURE, "Error while increasing the list of client network.c");

    
    for (size_t i = ptr->size / 2; i < ptr->size; i++)
        ptr->list[i].status = NOTUSED;
        
    pthread_mutex_unlock(&ptr->lockList);
}

size_t findNextNotUsed(struct listClientInfo *clients)
{
    size_t res = 0;
    while (res < clients->size && clients->list[res].status != NOTUSED)
    {
        res++;
    }
    
    if(res == clients->size && clients->list[res - 1].status != NOTUSED)
    {
        extendList(clients);
    } 

    return res;
}



/*
Creat un ptr for a struct clientInfo, set config to IPV4
*/

struct clientInfo * initClient(struct listClientInfo *clients)
{
    size_t res = findNextNotUsed(clients);
    struct clientInfo *client = &clients->list[res];

    client->status = NOTUSED;
    client->IP.sa_family = AF_INET;
    client->ID = rand() * rand();
    client->fd = -1;

    dup2(clients->fdin, client->fdinThread);
    dup2(clients->fdout, client->fdoutThread);

    return client;
}

void printIP(struct sockaddr *IP)
{
    char *buff = malloc(16 * sizeof(char));
    inet_ntop(AF_INET, &(((struct sockaddr_in *)IP)->sin_addr), buff, 16);
    printf("%s\n", buff);
    free(buff);
}

/*
    Remove a client without freeing it, it just change the status to NOUSED
*/

void removeClient(struct clientInfo client, struct listClientInfo *clients)
{
    size_t i = 0;
    pthread_mutex_lock(&clients->lockList);
    for (; i < clients->size && clients->list[i].ID != client.ID; i++)
    {
        ;
    }

    if (clients->list[i].ID == client.ID)
    {
        clients->list[i].status = NOTUSED;
    }
    else
    {
        printf("Error while removing, this client does not exist in the list of client\n");
    }
    
    pthread_mutex_unlock(&clients->lockList);
}
