#include "../src/configuration.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "format.h"
#include "global.h"
#include "Wurfl.h"


//read line from FILE. Ends when \n is found
void read_line(FILE *config,char *line)
{
	line[0]='\0';
	if(fgets(line, 256,config)!=NULL)
		return;
	return;
}

//Splits line when = character is found
//returns first word in line, second word in buf (arguments)
void parse_line(char *line,char *buf)
{
	buf[0]='\0';
	int len = (int) strlen(line);
	int i=0;
	bool found = false;
	while (i<len && !found)
	{
		if (line[i]=='=')
		{
			strcpy(buf, &line[i+1]);
			line[i]  = '\0';
			found = true;
		}
		i++;
	}
	return;
}

//Open configuration file, read and set global parameters
void config_server(void)
{
	_SERVER_ROOT=alloc_string();
	KEEP_ALIVE=alloc_string();
	DOCUMENT_ROOT=alloc_string();
	IM_PATH=alloc_string();
	CACHE_ROOT=alloc_string();
	WURFL_PATH=alloc_string();
	FILE* fd_conf;
	int int_value=0;
	fd_conf = open_file(_CONF_FILE,"r",MUST);
	char line[128],value[128];
	while (!feof(fd_conf))
	{
		read_line(fd_conf,line);
		parse_line(line,value);
		if (strcmp(line,"server_root")!=0 && strcmp(line,"cache_root")!=0 && strcmp(line,"document_root")!=0 && strcmp(line,"im_path")!=0)
			int_value=cast_to_int(value);
		if (strcmp(line,"server_root")==0)
		{
			strcpy(_SERVER_ROOT,value);
			_SERVER_ROOT[strlen(_SERVER_ROOT)-1]='\0';
		}
		else if (strcmp(line,"document_root")==0)
		{
			strcpy(DOCUMENT_ROOT,value);
			DOCUMENT_ROOT[strlen(DOCUMENT_ROOT)-1]='\0';
		}
		else if (strcmp(line,"cache_root")==0)
		{
			strcpy(CACHE_ROOT,value);
			CACHE_ROOT[strlen(CACHE_ROOT)-1]='\0';
		}
		else if (strcmp(line,"listen_port")==0)
			LISTEN_PORT = int_value;
		else if (strcmp(line,"keep_alive")==0)
		{
			strcpy(KEEP_ALIVE,value);
			KEEP_ALIVE[strlen(KEEP_ALIVE)-1]='\0';
		}
		else if (strcmp(line,"keep_alive_timeout")==0)
			KEEP_ALIVE_TIMEOUT = int_value;
		else if (strcmp(line,"max_clients")==0)
			MAX_CLIENTS = int_value;
		else if (strcmp(line,"cache_service")==0)
			CACHE_SERVICE= int_value;
		else if (strcmp(line,"im_path")==0)
		{
			strcpy(IM_PATH,value);
			IM_PATH[strlen(IM_PATH)-1]='\0';
		}
		else if (strcmp(line,"wurfl_path")==0)
		{
			strcpy(WURFL_PATH,value);
			WURFL_PATH[strlen(WURFL_PATH)-1]='\0';
		}
	}
	if (CACHE_SERVICE != 0)
		initialize_wurfl();
	close_file(fd_conf);
}


