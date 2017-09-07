#ifndef http_h   /* Include guard */
#define http_h

#include <stdio.h>
#include <stdbool.h>
#include "threadlist.h"

// thread_list pointer to the list
extern struct thread_list *list;

void startup(void);
void *start_thread(void *);
void accept_request(int ,int*);
int is_space(char);
void serve_file(int , char *,char*,int*);
int get_line(int , char *, int ,int*);
void unimplemented(int,char* );
void headers(int , char *,char*);
void not_found(int ,char*);
void get_mime_type(const char *,char*);
void cat(int , char *);
bool find_accept (char* ,char*);
bool check_mime_type(char *);
void parse_accept (char* ,char*,char*,char*);






#endif
