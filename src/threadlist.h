#ifndef threadlist_h
#define threadlist_h

#include <pthread.h>
#include <netinet/in.h>

extern pthread_mutex_t list_mtx;


//contains thread data for job
struct thread_node
{
	pthread_t tid;
	int thread_socket;
	struct sockaddr_in client_addr;
	struct thread_node *prev;
	struct thread_node *next;
};

//double linked list
//contains also server address
struct thread_list
{
    int count;
    struct thread_node *first;
    struct thread_node *last;
    struct sockaddr_in serv_addr;
};

struct thread_node* alloc_node(void);
void delete_node(struct thread_node* ,struct thread_list *);
void append_node(struct thread_node *,struct thread_list *);
struct thread_list* create_list(void);
void delete_list(struct thread_list*);
void print_list_tid(struct thread_list*);
void thread_list_mtx_lock(void);
void thread_list_mtx_unlock(void);


#endif
