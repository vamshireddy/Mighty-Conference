// First check wheather the handle client has already been added, if no then define HANDLE_CLIENT_H
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
	This thread will self destroy by calling this function.
	This is triggered because the client's connection is terminated.
*/

int delete_client_thread(int is_client_added_to_list)
{
	/* Now deletion of the client has two parts.
		 1) If the client has not yet logged in but terminated while reading username
		 or password ,then his node won't be present in the list, 
		 so we can directly delete
		 the thread by pthread_exit()

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
	char* len_str = get_length_str(string);
	// Now send the length string to client
	Write(client->clientfd,len_str,JSON_LEN_SIZE,client);
	// Now send the online clients string to the client
	Write(client->clientfd,string,strlen(string)+1,client);
}

void* client_function(void* a)
{
	// New thread
	args_list_t* args = (args_list_t*)a;

	// -------Arguments ---------//
	client_node_t* client  = NULL;
	int clientfd = args->clientfd;	  // Client socket descriptor
	int is_client_added_to_list = 0; // if the client is added to the list, then this flag will be set
	// --------------------------//

	printf("New thread (%u) created for the client (%d)\n", (unsigned int)pthread_self(), clientfd);

	// Read the username of 20 characters and password of 20 characters and authenticate the client

	// 1) Read the username
	printf("Client is entering username : ");
	char username[USERNAME_LENGTH];
	Read(clientfd, username, USERNAME_LENGTH, is_client_added_to_list);
	printf("\nThe username is %s\n",username);

	// 2) Read the password
	char password[PASSWORD_LENGTH];
	Read(clientfd, password, PASSWORD_LENGTH, is_client_added_to_list);
	printf("The password is %s\n",password);


	// 3) Validate the username and password
	while( authenticate(username,password) == 0 )
	{	
		// Status can be OKAY or DENY

		// Deny access as the credentials are invalid
		char* status = "DENY";
		printf("Client %s is trying to log in, the credentials are invalid!\n",username);
		Write(clientfd, status, LOGIN_STATUS_LENGTH, client);

		// Read username again
		Read(clientfd, username, USERNAME_LENGTH, is_client_added_to_list);
		printf("The username is %s\n",username);

		// Read password again
		Read(clientfd, password, PASSWORD_LENGTH, is_client_added_to_list);
		printf("The password is %s\n",password);
	}
	
	printf("Client %d logged in succesfully\n",username);
	// 4) Logged in succesfully
	// Now its time to create clients entry in the clients list
	// Create a client node and add it to the list
	client = create_new_client(args->clientfd, args->cliaddr, username);
	printf("New client created\n");

	// Add the client to the list and set the is_client_added_to_list = true(1)
	add_client(client, &is_client_added_to_list);
	printf("Client node created succesfully");

	// Send the status back to the client to let it access the application
	char* status = "OKAY";
	Write(clientfd, status, LOGIN_STATUS_LENGTH, client);

	// Now send all the current online clients in the list to the client
	send_clients_list(client);

	// ---------------------------------- NOW CONENCTION IS SET --------------------------------------------------- //


	// Now read for any messages sent by the client and perform the actions
	char message[JSON_LEN_SIZE];
	Read(clientfd, message, JSON_LEN_SIZE, is_client_added_to_list);
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


#endif /* HANDLE_CLIENT_H */
