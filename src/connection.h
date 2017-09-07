#ifndef connection_h
#define connection_h


void create_socket(void);
void shutdown_socket(int );
void bind_socket(void);
void listen_socket(void);
int accept_connection(struct sockaddr_in *);

#endif
