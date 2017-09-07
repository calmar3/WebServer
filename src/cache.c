#include "../src/cache.h"

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "file.h"
#include "format.h"
#include "global.h"
#include "log.h"


//Generate a conventional name of file to be stored in cache in format _name_q_w_h_dateLastModify
//Gets as argument the filename and q,w,h factors
char *normalize_file_name(char* path,char* q,char *w,char *h)
{
	char *buf =alloc_string();
	if (check_float(q) && !(check_int(h) && check_int(w)))
		sprintf (buf,"%s_%s",path,q);
	else if (!check_float(q) && (check_int(h) && check_int(w)))
		sprintf (buf,"%s_%s_%s",path,w,h);
	else if (check_float(q) && (check_int(h) && check_int(w)))
		sprintf (buf,"%s_%s_%s_%s",path,q,w,h);
	else
		strcpy(buf,path);
	replace_char(buf);
	char *orig_file_path=alloc_string();
	sprintf(orig_file_path,"%s%s",DOCUMENT_ROOT,path);
	sprintf(buf,"%s_%s",buf,get_last_modify(orig_file_path));
	return buf;
}

//running imagemagick with q,w,h if existing
//The arguments path(filename),q,w,h are used to create the command to execute
void exec_imagemagick(char* path,char* q,char *w,char *h)
{
	char * orig_file_path = alloc_string();
	char * cache_file_path = alloc_string();
	sprintf(orig_file_path,"%s%s",DOCUMENT_ROOT,path);
	sprintf(cache_file_path,"%s%s",CACHE_ROOT,normalize_file_name(path,q,w,h));
	if (check_float(q) && !(check_int(h) && check_int(w)))
	{
		char quality[4];
		sprintf(quality,"%1.f%%",100*cast_to_float(q));
		char *args[]= {IM_PATH,orig_file_path, "-quality",quality,cache_file_path, NULL };
		execv(IM_PATH,args);
	}
	else if (!check_float(q) && (check_int(h) && check_int(w)))
	{
		char *size=alloc_string();
		sprintf(size,"%sx%s",w,h);
		char *args[]= {IM_PATH,orig_file_path, "-resize",size,cache_file_path, NULL };
		execv(IM_PATH,args);
	}
	else if (check_float(q) && (check_int(h) && check_int(w)))
	{
		char *size=alloc_string();
		sprintf(size,"%sx%s",w,h);
		char quality[4];
		sprintf(quality,"%f%%",100*cast_to_float(q));
		char *args[]= {IM_PATH,orig_file_path,"-quality",quality,"-resize",size,cache_file_path, NULL };
		execv(IM_PATH,args);
	}
	return;
}

//returns the name of the image in the cache if it already exists with the same q,w,h arguments
//otherwise creates
void get_cache_file(char* filename,char*q,char*w,char*h,char * cache_file)
{
	char * path = normalize_file_name(filename,q,w,h);
	char * log_msg = alloc_string();
	sprintf(cache_file,"%s%s",CACHE_ROOT,path);
	if (!exists_file(cache_file))
	{
		pid_t pid = fork();
		if (pid==-1)
			error_die("fork");
		else if (pid == 0)
			exec_imagemagick(filename,q,w,h);
		else
		{
			sprintf(log_msg,"File %s NOT IN CACHE at q = %s  size = %sx%s. Execute image magick.",filename,q,w,h);
			write_cache_log(log_msg,"CACHE");
			waitpid(pid,0,0);
		}

	}
	else
	{
		sprintf(log_msg,"File %s IN CACHE at q = %s  size = %sx%s.",filename,q,w,h);
		write_cache_log(log_msg,"CACHE");
	}
}

