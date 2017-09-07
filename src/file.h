#ifndef file_h
#define file_h

#include <stdbool.h>
#include <stdio.h>


bool exists_file(char *);
char *get_last_modify(char*);
FILE* open_file(char*,char*,int);
void close_file(FILE*);
void get_file_size(char *,char *);
int open_fd(char *,int,int);
void close_fd(int);


#endif
//This header is used for files management
