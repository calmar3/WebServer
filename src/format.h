#ifndef format_h
#define format_h

#include <stdbool.h>

bool check_float(char*);
bool check_int(char*);
float cast_to_float(char *);
int cast_to_int(char *);
char *alloc_string(void);
void error_die(char *);
void replace_char(char* );
void get_current_time(char *,int);


#endif
//This header implements functions for management data
