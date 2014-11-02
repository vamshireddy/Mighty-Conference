/*
	This header file defines all the functions which are perfomed by the server when a 
	command is received in JSON message from the clients
*/

#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H
#include "common_headers.h"
#endif
#ifndef json_header
#define json_header
#include "json_utilities.h"
#endif

void parse_value(char* value,char* uname,char* pass)
{
	char username[USERNAME_LENGTH];
	char password[PASSWORD_LENGTH];
	int len = strlen(value);
	int i=0;
	// Obtain username now
	for(;i<len;i++)
	{
		if(value[i]=='$')
		{
			username[i] = '\0';
			i++; // skip $
			break;
		}
		username[i] = value[i];
		printf("%c\n",username[i]);
	}
	// Obtain password now
	int j=0;
	for(;i<len;i++)
	{
		if(value[i]=='\0')
		{
			password[j] = value[j];
			break;
		}
		password[j] = value[i];
		j++;
	}
	password[j] = '\0';
	printf("Username is %s and pass is %s\n",username,password);
	strcpy(uname,username);
	strcpy(pass,password);
}

/*
 This function will authenticate the client with the given
 username and password in the value string ( username$password )
 */
int handle_authentication(char* username,char* password)
{
	// Check the database and validate the username and password
	return 1;
}

/*
This function will reply the heartbeat message to the client
*/
char* handle_heartbeat(char* value)
{
	return JSON_make_str("HEARBEAT","BEEPBEEP");
}
