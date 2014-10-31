#include "common_headers.h"


/*

*/
parse_value(char* value,char* uname,char* pass)
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
	printf("UUUUUUUUUUUUUUU %s\n",username);
	printf("PASSSSSSSSSSSSSSSSS %s\n",password);
	printf("Username is %s and pass is %s",username,password);
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
void handle_heartbeat(char* value)
{

}
