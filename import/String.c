#include "String.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Returns the upper 
 */
void touppercase(char *str) {

	int i = 0;

	while(str[i])
	{
		str[i] = toupper(str[i]);
		i++;
	}
}

int contains_string(char *string, char *substring) {

	int i = 0;
	int flag = 0;
	
	char* token;
	token = strstr(string, substring);

	if(token != NULL) {
		flag = 1;
	}

	return flag;
}
