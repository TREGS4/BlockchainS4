#ifndef NETWORK_H
#define NETWORK_H

#include "network_tools.h"

/*

    PROBLEM: No mutext locking while modifing the clientlist could be dangerous

*/



/*
*   Network start the peer to peer network, it need at least server and hostname arguments,
*   the three last ones can be NULL.
*   server: pointer to a server structure initialised with initServer().
*
*   hostname: string containing the IP address or the hostname of the server, null terminated.
*
*   port: the port on wich the server is going to listen on. If null, the DEFAULT_PORT is used.
*
*   hostnameFirstServer: string containing the IP address or the hostname of one of the server on the
*   network, null terminated. Can be null.
*
*   portFirstServer: the port on wich we are going to contact hostnameFirstServer. If null, the 
*   DEFAULT_PORT is used.
*
*   return EXIT_FAILURE if a probem occurs, EXIT_SUCCESS otherwise.
*/
int Network(struct server *server, char *hostname, char *port, char *hostnameFirstServer, char *portFirstServer);

/*
*   Initalise a server structure and return its pointer.
*   If any error occurs return a null pointer.
*/
struct server *initServer(int api, int mining);

/*
*   Free properly the structure server pass in argument and all dynamic memory allocated in it. 
*/
void freeServer(struct server *server);

#endif
