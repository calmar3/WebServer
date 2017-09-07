#include "log.h"

#include <pthread.h>
#include <stdio.h>

#include "file.h"
#include "format.h"
#include "global.h"


FILE* cache_log;
FILE* server_log;
char* _SERVER_LOG ="/home/marco/WebServer/log/server_log.log";
char* _CACHE_LOG ="/home/marco/WebServer/log/cache_log.log";
pthread_mutex_t server_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cache_mtx = PTHREAD_MUTEX_INITIALIZER;

void lock_server_mtx(void)
{
	if (pthread_mutex_lock(&server_mtx)!=0)
		error_die("lock_server_mtx");
}

void unlock_server_mtx(void)
{
	if (pthread_mutex_unlock(&server_mtx)!=0)
		error_die("unlock_server_mtx");

}


void lock_cache_mtx(void)
{
	if (pthread_mutex_lock(&cache_mtx)!=0)
		error_die("lock_cache_mtx");

}


void unlock_cache_mtx(void)
{
	if (pthread_mutex_unlock(&cache_mtx)!=0)
		error_die("unlock_cache_mtx");

}

//Gets msg and error level as arguments, that must be written in server log
//Write also current time in log as a formatted string
void write_server_log(char *msg,char* error_level)
{
	char *date = alloc_string();
	lock_server_mtx();
	get_current_time(date,1);
	if(fprintf(server_log,"[%s] [%s] %s\n",date,error_level,msg)<0)
		error_die("write_server_log");
	fflush(server_log);
	unlock_server_mtx();
}

//Gets msg and error level as arguments, that must be written in cache log
//Write also current time as a formatted string
void write_cache_log(char * msg,char* error_level)
{
	char *date = alloc_string();
	lock_cache_mtx();
	get_current_time(date,1);
	if(fprintf(cache_log,"[%s] [%s] %s\n",date,error_level,msg)<0)
		error_die("write_cahce_log");
	fflush(cache_log);
	unlock_cache_mtx();

}

//open both log files in append mode
void open_log(void)
{
	server_log = open_file(_SERVER_LOG,"a",MUST);
	cache_log = open_file(_CACHE_LOG,"a",MUST);
}

//close both log files
void close_log(void)
{
	close_file(server_log);
	close_file(cache_log);
}






