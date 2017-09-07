#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "configuration.h"
#include "format.h"
#include "http.h"
#include "global.h"



//create LISTEN_SOCKET and set relative global parameter 
void create_socket(void)
{
	LISTEN_SOCKET = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (LISTEN_SOCKET == -1)
		error_die("socket");
}

//close socket file descriptor (argument)
void shutdown_socket(int fdSocket)
{
	if (shutdown(fdSocket,SHUT_RDWR)==-1)
		error_die("SHUT DOWN socket");
	return;
}

//bind on port LISTEN_PORT parameter set in config file
void bind_socket(void)
{
	memset((void *)&(list->serv_addr), 0, sizeof(list->serv_addr));
	list->serv_addr.sin_family = AF_INET;
	list->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	list->serv_addr.sin_port	   = htons(LISTEN_PORT);
	if ((bind(LISTEN_SOCKET, (struct sockaddr *)&(list->serv_addr), sizeof(list->serv_addr))) < 0)
		error_die("bind");
}

//set socket to listen status with BACKLOG VALUE = 50
void listen_socket(void)
{
	if (listen(LISTEN_SOCKET,50) < 0 )
		error_die("listen");
}

//Accept connection from LISTEN_SOCKET, get client address and store it in
//cliaddr (argument). Returns new socket with established connection
int accept_connection(struct sockaddr_in *cliaddr)
{
	unsigned int len;
	int connected_socket;
	len = sizeof(*cliaddr);
    if ((connected_socket = accept(LISTEN_SOCKET, (struct sockaddr *)cliaddr, &len)) < 0)
    	error_die("accept");
    return connected_socket;
}
