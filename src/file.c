#include "../src/file.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "format.h"

//return last modify date of the path passed by argument
char *get_last_modify(char*path)
{
	char *buffer = alloc_string();
	struct tm* clock;
	struct stat attrib;
	stat(path, &attrib);
	clock = localtime(&(attrib.st_mtime));
	strftime(buffer, 26, "%Y_%m_%d_%H_%M_%S",clock);
    replace_char(buffer);
    return buffer;
}

//check if path exists on disk
bool exists_file(char *path)
{
	struct stat st;
	if( stat(path, &st) == -1 )
		return false;
	else
		return true;
}
// returns a file pointer of the path passed by argument, opened with the specified mode and flag
// flag = 1 -> file must exists: flag = 0 -> file could not exists
FILE* open_file(char* path,char *mode,int flag)
{
	FILE *stream = fopen(path,mode);
	if (stream == NULL && flag == 1)
		error_die("fopen");
	return stream;
}


// flag = 1 -> file must exists: flag = 0 -> file could not exists
void close_file(FILE *stream)
{
	if(fclose(stream)!=0)
		error_die("fclose");
	return;
}

// returns a file descriptor of the path passed by argument, opened with the specified mode and flag
// flag = 1 -> file must exists: flag = 0 -> file could not exists
int open_fd(char* path,int mode,int flag)
{
	int file_fd;
	file_fd = open(path,mode);
	if(file_fd == -1 && flag ==1)
		error_die("open_fd");
	return file_fd;
}

// flag = 1 -> file must exists: flag = 0 -> file could not exists
void close_fd(int fd)
{
	if(close(fd)!=0)
		error_die("close_fd");
	return;
}

//returns file (filename) length in dest (argument)
void get_file_size(char *dest,char *filename)
{
	struct stat st;
	stat(filename, &st);
	sprintf(dest,"%ld",(long int) st.st_size);
}
