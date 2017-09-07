#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


struct thread_list *list;
#define SERVER_STRING "Server: WebServer/0.1\r\n"
#define GMT 0
#define LOCAL 1
#define MUST 1
#define NOT_MUST 0
char* _CONF_FILE;
char* _SERVER_ROOT;
int LISTEN_PORT;
char* KEEP_ALIVE;
int KEEP_ALIVE_TIMEOUT;
int MAX_CLIENTS;
int CACHE_SERVICE;
char* DOCUMENT_ROOT;
char* CACHE_ROOT;
char* IM_PATH;
char* WURFL_PATH;
int LISTEN_SOCKET;
