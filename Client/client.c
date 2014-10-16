#include "common_headers.h"
#include "socket_utilities.h"

char* read_JSON_string(int sockfd,char* str)
{
}

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

	// Send the user name and password to the server for authentication

	char username[USERNAME_LENGTH];
	scanf("%s",username);
	printf("Now sending username\n");
	Write(sock_fd,username,USERNAME_LENGTH);

	char password[PASSWORD_LENGTH];
	scanf("%s",password);
	printf("Now sending password\n");
	Write(sock_fd,password,20);

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

	// Client succesfully authenticated by the server

	// Now fetch the online clients

	char cli_str_JSON[1024];
	read_JSON_string(sock_fd,&cli_str_JSON);
	

}
