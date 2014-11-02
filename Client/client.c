#include "common_headers.h"
#include "socket_utilities.h"
#include "json_utilities.h"



int main()
{
	int sock_fd = socket(AF_INET,SOCK_STREAM,0);
	
	// Create the server socket
	struct sockaddr_in server_sock;
	// Initialize the socket structure to zero
	bzero(&server_sock,sizeof(struct sockaddr_in));
	printf("Enter the port number of server \n");
	int port;
	scanf("%d",&port);
	server_sock.sin_port = htons(port);
	server_sock.sin_family = AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&server_sock.sin_addr);

	connect(sock_fd,(struct sockaddr*)&server_sock,sizeof(struct sockaddr_in));

	printf("Enter your name : ");
	char name[USERNAME_LENGTH];
	scanf("%s",name);
	char str[40];
	sprintf(str,"{\"AUTH\":\"%s$not\"}",name);
	//char* str = "{\"AUTH\":\"$not\"}";
	int i;
	printf("Sending : %s\n\n",str);
	
	char* len_str = JSON_make_length_str(str);

	Write(sock_fd,len_str,JSON_LEN_SIZE);

	Write(sock_fd,str,strlen(str));


	while(1)
	{
		char len_str1[JSON_LEN_SIZE+1];
		Read(sock_fd,len_str1,JSON_LEN_SIZE);
		len_str1[JSON_LEN_SIZE] = 0; // Making it a string

		int len = atoi(JSON_get_value_from_pair(len_str1, "LENGTH"));

		char strrecv[len+1];
		Read(sock_fd,strrecv,len);
		strrecv[len] = 0;	// This is for making the char array as a string
		printf("RECIEVED STRING :  %s\n\n",strrecv);
	}


/*	// Send the user name and password to the server for authentication

	char username[USERNAME_LENGTH];
	scanf("%s",username);
	printf("Now sending username\n");
	Write(sock_fd,username,USERNAME_LENGTH);

	char password[PASSWORD_LENGTH];
	scanf("%s",password);
	printf("Now sending password\n");
	Write(sock_fd,password,PASSWORD_LENGTH);

	// Now get the authentication status

	// RIGHT NOW EVERYTHING IS A NORMAL STRING< >>> WE HAVE TO CONVERT THEM TO JSON LATER


	char status[LOGIN_STATUS_LENGTH];
	Read(sock_fd,status,LOGIN_STATUS_LENGTH);

	while( strcmp(status,"DENY") == 0 )
	{
		// Denied access, try again
		scanf("%s",username);
		printf("Now sending username\n");
		Write(sock_fd,username,USERNAME_LENGTH);

		scanf("%s",password);
		printf("Now sending password\n");
		Write(sock_fd,password,PASSWORD_LENGTH);
	}


	// Client is succesfully authenticated by the server

	char cli_strlen_JSON[JSON_LEN_SIZE];

	// Now fetch the length of the upcoming list
	Read(sock_fd,cli_strlen_JSON, JSON_LEN_SIZE);

	
	printf("%si\n",cli_strlen_JSON);

	// With this length build a new string to receive online clients list
	int length = get_length(cli_strlen_JSON);

	if( length <= 0 )
	{
		// Error in extracting length
		printf("Error in fetching the length\n");
		exit(0);
	}

	printf("The length is %d\n",length);

	char online_clients[length+1]; // +1 for providing space for null character
	Read(sock_fd,online_clients, length+1); // Read the null character too
	printf("The online clients are : \n%sEND\n",online_clients);


	// This loop is for continous reading of messages from the server, if any
	// Otherwise, it will simply wait at this prompt untill a msg is seen from the server.
	while( read(sock_fd,cli_strlen_JSON,JSON_LEN_SIZE) > 0 )
	{
		length =  get_length(cli_strlen_JSON);
		printf("New string with length %d\n",length);

		char clients_str[length+1];
		Read(sock_fd,clients_str, length+1);

		printf("%sEND\n",clients_str);
	}*/
}
