#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "src/configuration.h"
#include "src/connection.h"
#include "src/format.h"
#include "src/global.h"
#include "src/http.h"
#include "src/log.h"
#include "src/threadlist.h"



int main(int argc, char *argv[])
{
	int flag;
	char *date = alloc_string();
	get_current_time(date,1);
	//DEFAULT
	_CONF_FILE="/home/marco/WebServer/config/server.conf";
	fprintf(stderr,"Current time: %s\n",date);
	if (argc!=3)
	{
		fprintf(stderr,"bad usage. Expected %s <namefile> <flag>",argv[0]);
		return(EXIT_FAILURE);
	}
	else
	{
		_CONF_FILE=alloc_string();
		strcpy(_CONF_FILE,argv[1]);
	}
	flag = cast_to_int(argv[2]);
	config_server();
	open_log();
	startup();
	fprintf(stderr,"Web server running on port %d\n",LISTEN_PORT);
	fprintf(server_log,"Web server running on port %d\n",LISTEN_PORT);
	if (flag==0)
	{
		pid_t pid = fork();
		if (pid==-1)
			error_die("fork");
		if (pid!=0)
			return (EXIT_SUCCESS);
	}
	while(1)
	{
		while (list->count < MAX_CLIENTS)
		{
		struct thread_node *node = alloc_node();
		node->thread_socket = accept_connection(&(node->client_addr));
		append_node(node,list);
		if (pthread_create(&(node->tid),NULL,start_thread,(void*)node)!=0)
			error_die("pthread_create");
		}
	}
	close_log();
	delete_list(list);
	return EXIT_SUCCESS;
}

