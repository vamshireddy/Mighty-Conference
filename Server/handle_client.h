#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H
#include "common_headers.h"
#endif

#ifndef command_func_h
#define command_func_h
#include "command_functions.h"
#endif


#ifndef json_header
#define json_header
#include "json_utilities.h"
#endif

/*
	This is the arguments structure which is helpful in passing the arguments while creating a n	    ew thread
*/
typedef struct argument_list
{
	int clientfd;
	struct sockaddr_in* cliaddr;
}args_list_t;


/*
	This will be called by the thread which is allocated to a client.
	This thread will self destroy by calling this function.
	This is triggered because the client's connection is terminated.
*/
int delete_client_thread(int is_client_added_to_list)
{
	/* Now deletion of the client has two parts.
		 1) If the client has not yet logged in but terminated while reading username
		 or password,then his node won't be present in the list, 
		 so we can directly delete the thread by pthread_exit()

		 2) If the client has terminated after logging in, then we need to remove the node from the list and send a
		 Notification to all other clients.
	*/


	/* If the client's node is added in the list, then we have to delete it and send notifications to
	 all the other clients about the deletion, so that they can refresh their online clients list. */
	if( is_client_added_to_list != 0 )
	{
		// Node is in the list , so delete it
		remove_client(pthread_self());
		printf("Client has been removed\n");
	}
	printf("Current online list is \n");
	display_clients();
	// Now destroy itself.
	pthread_exit((void*)0);
}


/*
	Send the list of the clients who are logged in currently to client with clientfd.
*/
int send_clients_list(client_node_t* client)
{
	// Get the online clients and build the string from it
	char* string = build_JSON_string_from_list(client);
	// Get the length first of the returned string
	char* len_str = JSON_make_length_str(string);
	// Now send the length string to client
	Write(client->clientfd,len_str,JSON_LEN_SIZE,client);
	// Now send the online clients string to the client
	Write(client->clientfd,string,strlen(string),client);
}



int serve_command(int clientfd, char* command, client_node_t** client, int* is_client_added_to_list, struct sockaddr_in* sockaddr)
{
	// {"COMMAND_NAME":"value"}
	char* value = JSON_get_value_from_pair(command,"AUTH");
	if( value!= NULL )
	{
		// AUTHENTICATION PART
		char* auth_str;

		char username[USERNAME_LENGTH];
		char password[PASSWORD_LENGTH];
		parse_value(value,username,password);

		// Handle authentication from the value
		if( handle_authentication(username,password) == 1 )
		{
			// Access granted
			// Create a new node
			*client = create_new_client(clientfd, sockaddr, username);
			printf("New client created\n");


			// Add the node to the list
			add_client(*client, is_client_added_to_list);
			printf("Client node created succesfully\n");

			// Now send back reply to the client
			auth_str = JSON_make_str("AUTH","GRANTED");		
		}
		else
		{
			auth_str = JSON_make_str("AUTH","DENY");
		}
		// Send auth string len
		char* len_str = JSON_make_length_str(auth_str);
		Write(clientfd, len_str, JSON_LEN_SIZE , *client);
		// Send the auth string
		Write(clientfd, auth_str, strlen(auth_str), *client);

		// Now send online clients list to the current client
		send_clients_list(*client);
		// Done!!
	}
	else
	{
		value = JSON_get_value_from_pair(command,"HEARTBEAT");
		if( value!= NULL )
		{
			printf("%s\n", "Its a Heartbeat message");
			//  Invoke Heartbeat message handling function with the value
			handle_heartbeat(value);
		}
		else
		{
			printf("Error in the JSON string from client\n");
			// Error
		}
	}
}


void* client_function(void* a)
{
	// New thread
	args_list_t* args = (args_list_t*)a;

	// -------Arguments passed to thread---------//
	client_node_t* client  = NULL;
	int clientfd = args->clientfd;	  // Client socket descriptor
	int is_client_added_to_list = 0;  
	// if the client is authenticated, then it will be added to the list, then this flag will be set
	// --------------------------//

	printf("New thread (%u) created for the client (%d)\n", (unsigned int)pthread_self(), clientfd);


	char length_recv_buffer[JSON_LEN_SIZE];

	while( 1 ) // Listen for commands from the client
	{
		// Receive Length of the command from the client
		// is_client_added_to_list indicates wheather the client is there in the linked list 
		printf("IAMMMMM AT START AGAINB");
		Read(clientfd, length_recv_buffer, JSON_LEN_SIZE, is_client_added_to_list);
		printf("THIS IS RECV : %s\n",length_recv_buffer);
		

		// Get the length of the string that the client is about to send
		int len = atoi(JSON_get_value_from_pair(length_recv_buffer,"LENGTH"));
		
		// Allocate a buffer for the command
		char* command_buffer = (char*)malloc(sizeof(len+1)); // +1 for the null char
		
		// Now read for that length into the allocated buffer
		Read(clientfd, command_buffer, len, is_client_added_to_list);
		printf("THIS IS RECV : %s\n",command_buffer);
		
		// ----- Now the command is fetched ----- //
		serve_command(clientfd, command_buffer, &client, &is_client_added_to_list, args->cliaddr);
	}
}


/* 
	Main function will call this function to hand over a client to a new thread by spawning a new thread
*/
int handle_client_request(int clientfd, const struct sockaddr_in cliaddr)
{
	// Create a new thread for this client and return this function
	// Build the arguments structure
	args_list_t* args = (args_list_t*)malloc(sizeof(args_list_t));
	args->clientfd = clientfd;
	args->cliaddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

	memcpy(args->cliaddr, &cliaddr, sizeof(cliaddr));

	pthread_t tid;
	if( pthread_create(&tid, NULL, client_function, (void*)args) != 0)
	{
		printf("Failed to spawn a thread for the client");
		return -1;
	}
}
