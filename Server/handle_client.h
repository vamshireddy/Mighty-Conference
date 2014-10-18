#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include "common_headers.h"

/*
	This is the arguments structure which is helpful in passing the arguments while creating a new thread
*/
typedef struct argument_list
{
	int clientfd;
	struct sockaddr_in* cliaddr;
	clients_list_t* list;
}args_list_t;

/*
	This will authenticate a client with a username and password 
	TODO : Should contact the database, but for now its always valid
*/
int authenticate(char* username,char* password)
{
	return 1;
}

/*
	This will be called by the thread which is allocated to a client.
	This thread will self destroy by calling this function, as the client's connection is terminated
*/

int delete_client_thread(clients_list_t* list)
{
	// This thread is going to be deleted
	// First reclaim the memory

	// First remove its client from the online list
	remove_client(list, pthread_self());
	printf("Client has been removed\n");
	printf("Current online list is \n");
	display_clients(list);
	printf("\nNOW SENDING UPDATE TO EVERYONE ABOUT THE DELTION\n\n");
	// Now destroy itself.
	pthread_exit((void*)0);
}

void send_length_of_msg(int clientfd, char* str)
{
	int len = strlen(str);
	char len_str[LEN_STR_LENGTH];
	snprintf(len_str,6,"%5d",len);

	json_t* len_str_object = json_object();
  	json_object_set_new(len_str_object, "length", json_string(len_str));


	char* s = json_dumps(len_str_object, JSON_DECODE_ANY);

	printf("Built length str :%sEND and length is %d\n",s,strlen(s));

	// Send the length to the client
	Write(clientfd,s,JSON_LEN_SIZE);
}

/*
	Send the list of the clients who are logged in currently to client with clientfd.
*/
int send_clients_list(int clientfd, clients_list_t* list,client_node_t* client)
{
	char* string = build_JSON_string_from_list(list,client);
	
	// Send the length first
	send_length_of_msg(clientfd,string);

	// Now send the online clients
	write(clientfd,string,strlen(string)+1);
	printf("Written %sEND\n",string);
}

int read_str(int clientfd, char* buffer, int size, clients_list_t* list)
{
	int char_count = size;
	int chars_read = 0;
	while( ( chars_read = read(clientfd, buffer + chars_read , char_count ) ) > 0 )
	{
		char_count = char_count - chars_read;
		if( char_count == 0 )
		{
			// All chars are read, break out
			break;
		}
	}
	if( chars_read == -1 )
	{
		perror("Error in reading the line in readLine function : handle_client.h\n");
		return -1;
	}
	else if( chars_read == 0 )
	{
		printf("Client's connection is terminated\n");
		delete_client_thread(list);
		return 0;
	}
}

void* client_function(void* a)
{
	// New thread
	args_list_t* args = (args_list_t*)a;

	// -------Arguments ---------//
	clients_list_t* list = args->list;
	int clientfd = args->clientfd;
	// --------------------------//

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, (args->cliaddr), ip, INET_ADDRSTRLEN);

	printf("New thread (%u) created for the client ( %d ) and IP address = %s\n", (unsigned int)pthread_self(), args->clientfd, ip);

	
	// Create a client node
	client_node_t* client = create_new_client(args->clientfd, args->cliaddr);

	printf("New client created\n");


	// Validate the client's username and password
	// Read the username of 20 characters and password of 20 characters
	char username[USERNAME_LENGTH];
	read_str(clientfd, username, USERNAME_LENGTH, list);
	printf("The username is %s\n",username);

	// Copy the user name to the client node on the list
	strncpy(client->client_id,username,USERNAME_LENGTH);

	// Add the client to the list
	add_client(list, client);

	// Read the password
	char password[PASSWORD_LENGTH];
	read_str(clientfd, password, PASSWORD_LENGTH, list);
	printf("The password is %s\n",password);

	while( authenticate(username,password) == 0 )
	{	
		// Status can be OKAY or DENY
		char* status = "DENY";
		printf("Client %s is trying to log in, the credentials are invalid!\n",client->client_id);
		Write(clientfd,status,LOGIN_STATUS_LENGTH);

		// Read username again
		read_str(clientfd, username, USERNAME_LENGTH, list);
		printf("The username is %s\n",username);

		// Read password again
		read_str(clientfd, password, PASSWORD_LENGTH, list);
		printf("The password is %s\n",password);
	}
	
	// Logged in succesfully
	printf("Client %s logged in succesfully\n",client->client_id);
	char* status = "OKAY";
	Write(clientfd,status,LOGIN_STATUS_LENGTH);

	// Now send all online clients in the list to the client
	
	send_clients_list(clientfd,list,client);

}

int handle_client_request(clients_list_t* list, int clientfd, const struct sockaddr_in cliaddr)
{
	// Create a new thread for this client and return this function

	// Build the arguments structure
	args_list_t* args = (args_list_t*)malloc(sizeof(args_list_t));
	args->clientfd = clientfd;
	args->cliaddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	args->list = list;
	memcpy(args->cliaddr, &cliaddr, sizeof(cliaddr));

	pthread_t tid;
	if( pthread_create(&tid, NULL, client_function, (void*)args) != 0)
	{
		printf("Failed to spawn a thread for the client");
		return -1;
	}
}


#endif /* HANDLE_CLIENT_H */
