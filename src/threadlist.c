#include "threadlist.h"

#include <stdio.h>
#include <stdlib.h>
#include "format.h"


pthread_mutex_t list_mtx=PTHREAD_MUTEX_INITIALIZER;

//returns new node pointer
struct thread_node* alloc_node()
{
	struct thread_node *node;
	node = malloc(sizeof(struct thread_node));
	if (node==NULL)
		error_die("malloc thread_node");
	node->next=NULL;
	node->prev=NULL;
	return node;
}

//returns new empty list pointer
struct thread_list* create_list()
{
	struct thread_list *ptr;
	ptr = malloc(sizeof(struct thread_list));
	if (ptr==NULL)
		error_die("malloc thread_list");
	ptr->first=NULL;
	ptr->last=NULL;
	ptr->count=0;
	return ptr;
}

//destroy list (argument)
void delete_list(struct thread_list* list)
{
	if (list->count==1)
	{
		free(list->first);
		free(list->last);
		free(list);
	}
	struct thread_node *node=list->first,*next_node=node->next;
	for(node=list->first ; next_node != NULL ; node=node->next)
	{
		free(node);
		node=next_node;
		next_node=node->next;
	}
	free(node);
	free(list);
}

//delete node (argument) from list (argument). Decrease count value.
void delete_node(struct thread_node* node,struct thread_list *list)
{
	thread_list_mtx_lock();
	if (node == list->last && node == list->first)
	{
		list->first = NULL;
		list->last = NULL;
	}
	else if (node==list->first)
	{
		struct thread_node* new_first= node->next;
		list->first=new_first;
		new_first->prev=NULL;
		node->prev=NULL;
		node->next=NULL;
	}
	else if (node==list->last)
	{
		struct thread_node* new_last= node->prev;
		list->last=new_last;
		new_last->next=NULL;
		node->prev=NULL;
		node->next=NULL;
	}
	else
	{
		struct thread_node *prev=node->prev,*next=node->next;
		next->prev=prev;
		prev->next=next;
		node->prev=NULL;
		node->next=NULL;
	}
	free(node);
	list->count--;
	thread_list_mtx_unlock();
}

//append node (argument) in list(argument). Increase count value
void append_node(struct thread_node *node,struct thread_list *list)
{
	thread_list_mtx_lock();
	if (list->first==NULL && list->last==NULL)
	{
		list->first=node;
		list->last=node;
	}
	else
	{
		node->prev=list->last;
		list->last->next=node;
		list->last=node;
	}
	list->count++;
	thread_list_mtx_unlock();
}


void thread_list_mtx_lock(void)
{
	if (pthread_mutex_lock(&list_mtx)!=0)
		error_die("lock_list_mtx");
}

void thread_list_mtx_unlock(void)
{
	if (pthread_mutex_unlock(&list_mtx)!=0)
		error_die("unlock_list_mtx");
}

