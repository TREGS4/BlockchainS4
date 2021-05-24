#include "client.h"

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

	if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return EXIT_FAILURE;

	struct sockaddr_in IP = GetIPfromHostname(address); //return 0.0.0.0:00000 if the hostname is not valid
	struct sockaddr_in temp;
	memset(&temp, 0, sizeof(struct sockaddr_in));

	if (memcmp(&IP, &temp, sizeof(struct sockaddr_in)) == 0)
		return EXIT_FAILURE;

	if (connect(skt, (struct sockaddr *)&IP, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "Client: %s:%s disconnected\n", address.hostname, address.port);
		perror(NULL);
		return EXIT_FAILURE;
	}

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

	char *binMessage = MessageToBin(message);
	int res = Send(skt, binMessage, HEADER_SIZE + message->sizeData, 0);
	free(binMessage);
	res = close(skt) && res;
	return res;
}

/*
*	Take a list of client and message and send the message at each client
*	If there is a problem with the client, , it will remove of the list
*
*	Could be imrove with multithreading
*/
void SendMessage(struct clientInfo *clientList, MESSAGE *message)
{
	for (clientList = clientList->sentinel->next; clientList->isSentinel == FALSE; clientList = clientList->next)
	{
		if (SendMessageForOneClient(clientList, message) == EXIT_FAILURE)
		{
			struct clientInfo *temp = clientList;
			clientList = clientList->next;
			removeClient(temp);
		}
	}
}
