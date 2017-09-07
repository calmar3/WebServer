#ifndef log_h   /* Include guard */
#define log_h

#include <pthread.h>
#include <stdio.h>

extern FILE *cache_log;
extern FILE *server_log;
extern char* _SERVER_LOG;
extern char* _CACHE_LOG;
extern pthread_mutex_t server_mtx,cache_mtx;


void open_log(void);
void close_log(void);
void write_cache_log(char*,char*);
void write_server_log(char*,char*);
void lock_server_mtx(void);
void unlock_server_mtx(void);
void lock_cache_mtx(void);
void unlock_cache_mtx(void);


#endif
