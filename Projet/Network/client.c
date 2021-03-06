#include "client.h"

struct msgInfo
{
	MESSAGE *msg;
	struct clientInfo *client;
	int res;
	pthread_t thread;
};

/*
*	Secure the sending of message on a socket
*/
int Send(int fd, const void *buf, size_t count, int flag)
{
	ssize_t r = 0;
	size_t reste = count;

	while (r >= 0 && (r = send(fd, buf + count - reste, count, flag)) < (long int)reste)
		reste -= r;

	if (r < 0)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

/*
*	From a address structure create a socket and return it.
*/
int connectClient(struct address address)
{
	int skt = -1;
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return EXIT_FAILURE;

	if (setsockopt(skt, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		return EXIT_FAILURE;

	struct sockaddr_in *IP = GetIPfromHostname(address);

	if (IP == NULL || connect(skt, (struct sockaddr *)IP, sizeof(struct sockaddr_in)) < 0)
	{
		if (IP != NULL)
			free(IP);
		fprintf(stderr, "Client: %s:%s disconnected\n", address.hostname, address.port);
		return EXIT_FAILURE;
	}

	free(IP);
	return skt;
}

/*
*	Take a client to contact and the message as argument.
*	Connect to the client and the message.
*	return EXIT_FAILURE, if the client is unreachable or a problem occurs
*
*	Improvment ?
*	reveive directly the message in binary from the parent function
*	could be a little tricky if it's in an thread
*/

int SendMessageForOneClient(struct clientInfo *client, MESSAGE *message)
{
	int skt = -1;

	if ((skt = connectClient(client->address)) == EXIT_FAILURE)
		return EXIT_FAILURE;

	BYTE *binMessage = MessageToBin(message);
	int res = Send(skt, binMessage, HEADER_SIZE + message->sizeData, 0);
	free(binMessage);
	res = close(skt) && res;
	return res;
}

void *SendMessageThread(void *arg)
{
	struct msgInfo *temp = arg;
	temp->res = SendMessageForOneClient(temp->client, temp->msg);

	return NULL;
}

/*
*	Take a list of client and message and send the message at each client
*	If there is a problem with the client, it will remove of the list
*
*	Could be improve with multithreading
*/
void SendMessage(struct clientInfo *clientList, MESSAGE *message)
{
	size_t len = listLen(clientList);
	size_t i = 0;
	struct msgInfo *tab = malloc(sizeof(struct msgInfo) * len);
	if (tab == NULL)
		return ;

	for (clientList = clientList->sentinel->next; clientList->isSentinel == FALSE; clientList = clientList->next)
	{
		tab[i].msg = message;
		tab[i].client = clientList;
		tab[i].res = -2;

		if (pthread_create(&tab[i].thread, NULL, SendMessageThread, (void *)&tab[i]) != 0)
			tab[i].res = ERROR;
			
		i++;
	}

	i = 0;

	for (clientList = clientList->sentinel->next; clientList->isSentinel == FALSE; clientList = clientList->next)
	{
		if (tab[i].res == -2 || tab[i].res == EXIT_SUCCESS || tab[i].res == EXIT_FAILURE)
		{
			pthread_join(tab[i].thread, NULL);

			if (tab[i].res == EXIT_FAILURE)
			{
				struct clientInfo *temp = clientList;
				clientList = clientList->next;
				removeClient(temp);
			}
		}
		i++;
	}

	free(tab);
}
