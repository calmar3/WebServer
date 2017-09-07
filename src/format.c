#include "../src/format.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//ascii codes of chars that must be replaced
char invalid_char[]={
	47, // 		/ slash
	92, // 		\ backslash
	32, //		space
	58	//		:
};

//check if q (argument) has valid value (0<q<1)
//used for q factor quality
bool check_float(char* q)
{
	if (q==NULL)
		return false;
	if (0<cast_to_float(q) && cast_to_float(q)<1)
		return true;
	return false;
}

//check if w (argument) has valid value (0<w)
//used for width and height
bool check_int(char* w)
{
	if (w == NULL)
		return false;
	if (0<cast_to_int(w))
		return true;
	return false;
}

//convert string value(argument) to float for checking the value
float cast_to_float(char *value)
{
    char *p;
    errno = 0;
    float v = (float) strtof(value, &p);
    if (errno != 0 && *p != '\0')
    	error_die("strtof");
    return v;
}

//convert string value (argument) to int for checking the value
int cast_to_int(char *value)
{
    char *p;
    errno = 0;
    int v = (int) strtol(value, &p, 0);
    if (errno != 0 && *p != '\0')
    	error_die("strtol");
    return v;
}

//returns pointer to new string allocated of 128 characters
char * alloc_string()
{
	char *p = malloc(128*sizeof(char));
	if (p==NULL)
		error_die("malloc");
	p[0] = '\0';
	return p;
}

//prints error string (argument) and kill all processes
void error_die(char *error)
{
	perror(error);
	exit(EXIT_FAILURE);
}

//replaces invalid chars, token from invalid_chars array, of word(argument), with _
void replace_char(char* word)
{
	int i,j,len=strlen(word),len_array=strlen(invalid_char);
	for (i=0;i<len;i++)
	{
		for (j=0;j<len_array;j++)
		{
			if (word[i]==invalid_char[j])
				word[i]='_';
		}
	}
}

//returns current time in date (argument) using RFC1123 format
//if flag is setted as 0 returns GMT current time
//if flag is setted as 1 returns local current time
void get_current_time(char *date,int flag)
{
	const int RFC1123_TIME_LEN = 29;
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime );
	if (flag==0)
	{
		timeinfo = gmtime ( &rawtime );
		strftime(date, RFC1123_TIME_LEN+1, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	}
	else if (flag==1)
	{
		timeinfo = localtime(&rawtime);
		strftime(date, 26, "%Y-%m-%d %H:%M:%S", timeinfo);
	}
}

