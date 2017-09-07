#include "http.h"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "cache.h"
#include "configuration.h"
#include "connection.h"
#include "file.h"
#include "format.h"
#include "global.h"
#include "log.h"
#include "Wurfl.h"


/*
	search the header "User-agent:" in the string pointed by user_agent passed as argument,
	and if it's present copy the relative value in the string pointed by buf
*/
void find_user_agent(char* user_agent, char*buf)
{
	char *temp=alloc_string();
	strcpy(temp,buf);
	temp[11]='\0';
	if (strcmp(temp,"User-Agent:")==0)
		strcpy(user_agent,buf+12);


}

/*
	search the header "Accept:" in the string pointed by accept passed as argument,
	and if it's present copy the relative value in the string pointed by buf.
	If the header is present return true
*/
bool find_accept (char* buf,char*accept)
{
	char *temp=alloc_string();
	strcpy(temp,buf);
	temp[7]='\0';
	if (strcmp(temp,"Accept:")==0)
	{
		strcpy(accept,buf);
		return true;
	}
	else
		return false;
}


/*
	parse the Accept header value pointed by accepth and set the relative values of
		q - quality value of the requested resource
		h - heigth value in pixel of the requested resource
		w - width value in pixel of the requested resource
	passed as arguments
*/
void parse_accept (char* accepth,char *q, char *w, char *h)
{
	if (strstr(accepth,"q="))
	{
		strcpy(q,strstr(accepth,"q="));
		char *q_value = alloc_string();
		parse_line(q,q_value);
		float fq = cast_to_float(q_value);
		sprintf(q_value,"%.1f",fq);
		strcpy(q,q_value);
	}
	if (strstr(accepth,"h="))
	{
		strcpy(h,strstr(accepth,"h="));
		char *h_value = alloc_string();
		parse_line(h,h_value);
		int ih = cast_to_int(h_value);
		sprintf(h_value,"%d",ih);
		strcpy(h,h_value);
	}
	if (strstr(accepth,"w="))
	{
		strcpy(w,strstr(accepth,"w="));
		char *w_value = alloc_string();
		parse_line(w,w_value);
		int iw = cast_to_int(w_value);
		sprintf(w_value,"%d",iw);
		strcpy(w,w_value);
	}
}

/*
	check if the MIME Type string pointed by mime_type contains a valid value for the images types.
*/
bool check_mime_type(char *mime_type)
{
	char *temp=alloc_string();
	strcpy(temp,mime_type);
	temp[5]='\0';
	if (strcmp(temp,"image")==0)
		return true;
	else
		return false;
}

/*
	get the MIME Type of the resource by the path string pointed by filename.
	the result is set into the string pointed by mime
*/
void get_mime_type(const char *filename,char *mime)
{
	  FILE *fp;
	  char *command = alloc_string();
	  sprintf(command,"mimetype -b %s",filename);
	  fp = popen(command, "r");
	  if (fp == NULL)
		  error_die("cannot execut mime");
	  fscanf(fp, "%s", mime);
	  pclose(fp);
}

/*
	initialize the startup sequence of the HTTP web server
*/
void startup(void)
{
	//decidiamo dove metterla
	signal(SIGPIPE, SIG_IGN);
	create_socket();
	list=create_list();
	bind_socket();
	listen_socket();
}

/*
	void function called in the pthread_create.
	there is a thread per connection.
	each thread set the timeout parameter on the transfer socket in send and receive
	prepare the parameters used for tranfer in the struct thread_node
	each tread is wainting for icoming request on the tranfer socket
	if timeout occurs on the thread transfer socket, this is closed and the node in the thread_list is removed
*/
void *start_thread(void *ptr)
{
	struct thread_node *node = (struct thread_node*) ptr;
	char msg[128];
	struct timeval timeout;
	timeout.tv_sec = KEEP_ALIVE_TIMEOUT;
	timeout.tv_usec = 0;
	if (setsockopt (node->thread_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
		perror("setsockopt failed\n");
	if (setsockopt (node->thread_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
		perror("setsockopt failed\n");
	char * client_address =alloc_string(),*server_address =alloc_string();
	strcpy(client_address,inet_ntoa(node->client_addr.sin_addr));
	strcpy(server_address,inet_ntoa(list->serv_addr.sin_addr));
	sprintf(msg,"[%08x] %s:%d %s:%d connected to socket:%d",(unsigned int)node->tid , client_address , ntohs(node->client_addr.sin_port), server_address , ntohs(list->serv_addr.sin_port),node->thread_socket);
	write_server_log(msg,"NETW");
	int cond=0;
	while (cond==0)
		accept_request(node->thread_socket,&cond);
	shutdown(node->thread_socket,SHUT_RDWR);
	delete_node(node,list);
	return NULL;
}


/*
	read from the socket sock until the \n escape character occurs or no data is received.
	write the corrispondent data into buf by block of size dimension
*/
int get_line(int sock, char *buf, int size,int *cond)
{
	int i = 0,n;
	char c = '\0';
	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
		{
			c = '\n';
			*cond=1;
		}
	}
	buf[i] = '\0';
	return(i);
}


/*
	build and send and 501 Unimplemented HTTP message on the socket client.
	the response code 501 is set in the string pointed by response
*/
void unimplemented(int client,char* response)
{
	char buf[1024];
	sprintf(buf, "HTTP/1.1 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(response,"501");
}

/*
	copy by block size of 8096 bytes the binary content of the resource pointed by resource path
	and write in the client socket
*/
void cat(int client, char *resource)
{
	int  file_fd;
	long  ret;
	long BUFFER_SIZE = 8096;
	char * buffer = malloc(BUFFER_SIZE*sizeof(char*)); /* static so zero filled */
	memset(buffer,0,sizeof(buffer)*BUFFER_SIZE);
	file_fd = open_fd(resource,O_RDONLY,NOT_MUST);  /* open the file for reading */

	/* send file in 8KB block - last block may be smaller */
	while (	(ret = read(file_fd, buffer, BUFFER_SIZE)) > 0 ) {
		send(client,buffer,ret,0);

	}
	close_fd(file_fd);

}




/*
	build and send to the client socket the HEAD message reply for the resource pointed by filename
	set the responce code 200 into the string pointed by responce
*/
void headers(int client, char *filename,char*response)
{
	char buf[4096];
	char *mime_type = alloc_string();
	char *temp = alloc_string();
	strcpy(buf, "HTTP/1.1 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	get_current_time(temp,0);
	sprintf(buf,"Date: %s\r\n",temp); // obbligatorio
	send(client,buf,strlen(buf),0);
	sprintf(buf, "Connection: Keep-Alive\r\nKeep-Alive: timeout=%d, max=20\r\n",KEEP_ALIVE_TIMEOUT);
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "Accept-Ranges: bytes\r\n");
	get_mime_type(filename,mime_type);
	sprintf(buf, "Content-Type: %s\r\n",mime_type);
	send(client, buf, strlen(buf), 0);
	get_file_size(temp,filename);
	sprintf(buf, "Content-Length: %s\r\n",temp);
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(response,"200");
}

/*
	build and send to the client socket the 404 Not Found message reply
	set the responce code 404 into the string pointed by responce
*/
void not_found(int client,char *response)
{
	char buf[1024];
	sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(response,"404");
}


/*
	send in the client socket the HTTP resource reply message standard of the resource pointed by URL string
	if the cache service is enabled and JPEG image resource, the cached version is served with the relative value of
		q - quality value of the requested resource
		h - heigth value in pixel of the requested resource
		w - width value in pixel of the requested resource
	the values of h and w is set by the wurfle device capabilities
	the relative response code is set into response string
*/
void serve_file(int client,char *url,char *response,int *cond)
{
	FILE *resource;
	char *filename = alloc_string();
	sprintf(filename,"%s%s",DOCUMENT_ROOT,url);
	if (CACHE_SERVICE !=0)
	{
		int numchars = 1;
		char *q=alloc_string();
		char *h=alloc_string();
		char*w=alloc_string();
		char *user_agent=alloc_string();
		char *accepth = alloc_string();
		char buf[1024];
		buf[0] = 'A'; buf[1] = '\0';
		char *mime_type=alloc_string();
		get_mime_type(filename,mime_type);
		while ((numchars > 0) && strcmp("\n", buf))
		{
			numchars = get_line(client, buf, sizeof(buf),cond);
			if (check_mime_type(mime_type))
			{
				if (find_accept(buf,accepth))
					parse_accept(accepth,q,w,h);
				find_user_agent(user_agent,buf);
			}
		}
		if (check_mime_type(mime_type))
		{
			get_wurfl_values(user_agent,w,h);
			if (check_float(q) || (check_int(w) && check_int(h)))
				get_cache_file(url,q,w,h,filename);
		}
	}
	if (filename[strlen(filename) - 1] == '/')
		strcat(filename, "index.html");
	resource = open_file(filename,"r",NOT_MUST);
	if (resource == NULL)
		not_found(client,response);//404
	else
	{
		headers(client, filename,response);//200
		cat(client, filename);
	}
	close_file(resource);
}
/*
	main function executed by each thread
	parse the HTTP request message read on the client socket setting the values of
		HTTP method: 	GET | POST | HEAD
		URL string resource path
		URL query parameters
		indexing in case of directory resource
	the relative HTTP reply message is sent on the client socket in case of existing or no existing file in the WWW directory
*/
void accept_request(int client,int *cond)
{
	 char buf[1024];
	 char *response_code=alloc_string();
	 int numchars;
	 char method[255];
	 char url[255];
	 char path[512];
	 size_t i, j;
	 struct stat st;
	 char *query_string = NULL;
	 numchars = get_line(client, buf, sizeof(buf),cond); // get first line of header
	 i = 0; j = 0;
	 while (!is_space(buf[j]) && (i < sizeof(method) - 1))
	 {
		method[i] = buf[j];
		i++; j++;
	 }
	 method[i] = '\0';
	 if (strcasecmp(method, "GET") && strcasecmp(method, "POST") && strcasecmp(method,"HEAD"))
	 {
		unimplemented(client,response_code);
		return;
	 }
	 i = 0;
	 while (is_space(buf[j]) && (j < sizeof(buf)))
		 j++;
	 while (!is_space(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	 {
		 url[i] = buf[j];
		 i++; j++;
	 }
	 url[i] = '\0';
	 if (strcasecmp(method, "GET") == 0)
	 {
		  query_string = url;
		  while ((*query_string != '?') && (*query_string != '\0'))
			  query_string++;
		  if (*query_string == '?')
		  {
			  *query_string = '\0';
			  query_string++;
		  }
	 }
	 sprintf(path, "%s%s",DOCUMENT_ROOT, url);
	 if (path[strlen(path) - 1] == '/')
		 strcat(path, "index.html");
	 if (strcasecmp(method, "HEAD") == 0)
	 {
		headers(client,path,response_code);
		return;
	 }
	 if (stat(path, &st) == -1)
	 {
		 while ((numchars > 0) && strcmp("\n", buf))/* read & discard headers */
		 	 numchars = get_line(client, buf, sizeof(buf),cond);
		 not_found(client,response_code);

	 }
	 else
	 {
		 if ((st.st_mode & S_IFMT) == S_IFDIR)
			 strcat(path, "index.html");
		 if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH) )
		 {
			 serve_file(client, url,response_code,cond);
		 }
	 }
	 sprintf(buf,"%s %s %s %s to socket: %d",method,url,"HTTP/1.1",response_code,client);
	 write_server_log(buf,"INFO");
}

/*
	check if the char passed as argument is the SPACE character
*/
int is_space(char s)
{
	int i = (int)s;
	return isspace(i);
}
