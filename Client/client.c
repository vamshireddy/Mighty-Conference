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

	char cli_strlen_JSON[JSON_LEN_SIZE];

	// Now fetch the length of the upcoming list
	Read(sock_fd,cli_strlen_JSON, JSON_LEN_SIZE);

	
	printf("%si\n",cli_strlen_JSON);

	// With this length build a new string to receive online clients list
	int length = get_length(cli_strlen_JSON);

	if( length <= 0 )
	{
		// Error
		printf("Error in fetching the length\n");
		exit(0);
	}

	printf("The length is %d\n",length);

	char online_clients[length+1]; // +1 for providing space for null character
	Read(sock_fd,online_clients, length+1);
	printf("The online clients are : \n%sEND\n",online_clients);

	while( read(sock_fd,cli_strlen_JSON,JSON_LEN_SIZE) > 0 )
	{
		length =  get_length(cli_strlen_JSON);
		printf("New string with length %d\n",length);

		char clients_str[length+1];
		Read(sock_fd,online_clients, length+1);

		printf("%sEND\n",cli_strlen_JSON);
	}

}


int get_length(char* cli_strlen_JSON)
{
	// Extract the JSON part of the string
	json_error_t error;
	json_t* root = json_loads(cli_strlen_JSON, 0, &error);

	// Get the length value
	json_t* len_value_JSON = json_object_get(root, "length");

	char* len_text = json_string_value(len_value_JSON);

	// Return the length value string
	return atoi(len_text);

}