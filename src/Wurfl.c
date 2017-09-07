#include "../src/Wurfl.h"

#include <stdio.h>
#include <string.h>

#include "format.h"
#include "file.h"
#include "lib-wurfl/device.h"
#include "lib-wurfl/wurfl.h"
#include "global.h"

wurfl_t *wurfl;
/*
 	 void function to initializate wurfl "object"
 */
void initialize_wurfl(void)
{
	const char* patches[] = {NULL};
	wurfl = wurfl_init(WURFL_PATH, patches);
}

/*
 	 gets resolution width and resolution height from wurfl.xml matching User-Agent header
 	 passed as argument. Return both values in char* passed as arguments w,h
 */
void get_wurfl_values(char* user_agent,char* w,char* h)
{
	device_t* device = wurfl_match(wurfl, user_agent);
	char** capabilities = device_capabilities(device, NULL);
	char** caps_ptr = capabilities;
	while(caps_ptr!=NULL && *caps_ptr!=NULL) {
		if (strcmp(*caps_ptr,"resolution_width")==0)
			sprintf(w,"%s",*(caps_ptr + 1));
		else if (strcmp(*caps_ptr,"resolution_height")==0)
			sprintf(h,"%s",*(caps_ptr + 1));
		caps_ptr+=2;
	}
}

