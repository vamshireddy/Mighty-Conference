#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H
#include "common_headers.h"
#endif

#ifndef command_func_h
#define command_func_h
#include "command_functions.h"
#endif

#ifndef client_list_h
#define client_list_h
#include "client_list.h"
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
int self_delete_client_thread(int is_client_added_to_list)
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
	json_t* json_list_object = build_JSON_string_from_list(client);
	char* string = json_dumps(json_list_object, JSON_DECODE_ANY);
	// Get the length json object first
	json_t* json_object = JSON_make_length_str(string);
	// Make the length string
	char* len_str = json_dumps(json_object, JSON_DECODE_ANY);
	// Now send the length string to client
	Write(client->clientfd,len_str,JSON_LEN_SIZE,client);
	// Now send the online clients string to the client
	Write(client->clientfd,string,strlen(string),client);
	// Free up json object
	json_decref(json_object);
	json_decref(json_list_object);
}



int serve_command(int clientfd, char* command, client_node_t** client, int* is_client_added_to_list, struct sockaddr_in* sockaddr)
{
	// {"COMMAND_NAME":"value"}
	char* value;
	json_t* json_value_obj;

	// Extract the AUTH value and make a string out of it
	json_value_obj = JSON_get_value_from_pair(command,"AUTH");
	value = json_string_value(json_value_obj);

	printf("\n\n\nCLIENT VALUE IS %d\n\n",*is_client_added_to_list);

	if( value != NULL )
	{
		// AUTHENTICATION PART
		char* auth_str;
		json_t* auth_str_reply_o;
		char username[USERNAME_LENGTH];
		char password[PASSWORD_LENGTH];
		parse_value(value,username,password);

		// Handle authentication from the value
		if( handle_authentication(username,password) == 1 )
		{
			// Access granted
			// Before allocating a new client, check whether an entry is already present
			if( check_client(username) == 1 )
			{
				// User already in the list
				printf("Client node already present\nALREADY LOGGED IN\n");
				auth_str_reply_o = JSON_make_str("AUTH","ALREADYLOGGED");
			}
			else
			{	
				// User not in the list
				// Create a new node
				*client = create_new_client(clientfd, sockaddr, username);
				printf("New client created\n");

				// Add the node to the list
				add_client(*client, is_client_added_to_list);
				printf("Client node created succesfully\n");

				// Now send back reply to the client
				auth_str_reply_o = JSON_make_str("AUTH","GRANTED");	
			}
		}
		else
		{
			auth_str_reply_o = JSON_make_str("AUTH","DENY");		
		}
		// Dump the auth string from json object
		auth_str = json_dumps(auth_str_reply_o, JSON_DECODE_ANY);	
		// Send auth string len
		// Fetch the JSON string length object
		json_t* json_obj = JSON_make_length_str(auth_str);

		// Extract the string from the JSON object
		char* len_str = json_dumps(json_obj, JSON_DECODE_ANY);


		Write(clientfd, len_str, JSON_LEN_SIZE , *client);
		// Send the auth string
		Write(clientfd, auth_str, strlen(auth_str), *client);

		// Now update the client Node
		// Update the last contacted time in the client node
		if( *client != NULL )
		{
			printf("I am here");
			// Handling If the client is not created due to already logged in error
			time_t cur_time;
			time(&cur_time);
			(*client)->last_contacted_time = cur_time;
			
			// Now send online clients list to the current client
			send_clients_list(*client);
		}
		// Done!!
		// Free up the JSON objects
		json_decref(json_obj);
		json_decref(auth_str_reply_o);
	}
	else
	{
		// Extract the HEARTBEAT From the JSON object and make string out of it!
		json_value_obj = JSON_get_value_from_pair(command,"HEARTBEAT");
		if( json_value_obj == NULL )
		{
			printf("Error in JSON\n");
			pthread_exit((void*)0);
		}
		value = json_string_value(json_value_obj); 
	
		if( value != NULL && (*client != NULL))
		{
			printf("%s\n", "Its a Heartbeat message");
			//Invoke Heartbeat message handling function with the value
			json_t* heart_beat_reply_o = JSON_make_str("HEARBEAT","BEEPBEEP");
			char* heart_beat_reply = json_dumps(heart_beat_reply_o, JSON_DECODE_ANY);
			// Now update the client Node
			// Update the last contacted time in the client node
			time_t cur_time;
			time(&cur_time);
			(*client)->last_contacted_time = cur_time;

			// Construct a reply and send it back
			// Make length string
			json_t* json_obj = JSON_make_length_str(heart_beat_reply);
			if( json_obj == NULL )
			{
				printf("Error in Making JSON string\n");
				pthread_exit((void*)0);
			}
			// Extract the string from the JSON object
			char* len_str = json_dumps(json_obj, JSON_DECODE_ANY);
			if( len_str == NULL )
			{
				printf("Error in Making JSON string\n");
				pthread_exit((void*)0);
			}

			Write(clientfd, len_str, JSON_LEN_SIZE ,*client);
			// Send the hearbeat reply string
			Write(clientfd, heart_beat_reply, strlen(heart_beat_reply), *client);
			// Free up the strings
			printf("HEARTBEAT BEEP BEEP\n");
			json_decref(json_obj);
			json_decref(heart_beat_reply_o);
		}
		else
		{
			// Extract the REQUEST from the JSON object and make string out of it!
			/*json_value_obj = JSON_get_value_from_pair(command,"REQUEST");
			if( json_value_obj == NULL )
			{
				printf("Error in JSON COMMAND\n");
				pthread_exit((void*)0);
			}
			value = json_string_value(json_value_obj);
			if( value!=NULL && (*client != NULL))
			{
				// Received REQUEST Message
			}
			else
			{
				// Error format
				// Do something else
			}*/
		}
		
	}

	// Now free the json objects.
	if( json_value_obj != NULL )
	{
		json_decref(json_value_obj);
	}
	// TODO :
	// 1. Implement request message which comes from the stream initiator
	// 2. Implement response message from the stream acceptor.
}

void thread_cleanup(void* arg)
{
	int sockfd = (int)arg;
	/* Close the socket descriptor */
	close(sockfd);
}

void* client_function(void* a)
{
	// New thread
	args_list_t* args = (args_list_t*)a;

	// -------Arguments passed to thread---------//
	client_node_t* client  = NULL;
	int clientfd = args->clientfd;	  // Client socket descriptor
	int is_client_added_to_list = 0;  // Authenticated status
	// if the client is authenticated, then it will be added to the list, then this flag will be set
	// --------------------------//

	// -----------------------------------------------------------------------------------------
	//----------------------------Assign cleanup handlers---------------------------------------
	//------------------------------------------------------------------------------------------
	pthread_cleanup_push(thread_cleanup,(void*)clientfd);
	//------------------------------------------------------------------------------------------

	printf("New thread (%u) created for the client (%d)\n", (unsigned int)pthread_self(), clientfd);


	char length_recv_buffer[JSON_LEN_SIZE];

	while( 1 ) // Listen for commands from the client
	{
		// Receive Length of the command from the client
		// is_client_added_to_list indicates wheather the client is there in the linked list 
		Read(clientfd, length_recv_buffer, JSON_LEN_SIZE, is_client_added_to_list);

		// Get the length of the string that the client is about to send
		json_t* json_object = JSON_get_value_from_pair(length_recv_buffer,"LENGTH");
		int len = atoi(json_string_value(json_object));
		// Free up JSON object
		json_decref(json_object);

		// Allocate a buffer for the command
		char* command_buffer = (char*)malloc(sizeof(len+1)); // +1 for the null char
		
		// Now read for that length into the allocated buffer
		Read(clientfd, command_buffer, len, is_client_added_to_list);
		
		// ----- Now the command is fetched ----- //
		serve_command(clientfd, command_buffer, &client, &is_client_added_to_list, args->cliaddr);

		printf("Client AUTH STATUS : %d\n",is_client_added_to_list);
	}

	// -----------------------------------------------------------------------------------------
	//----------------------------Assign cleanup handlers---------------------------------------
	//------------------------------------------------------------------------------------------
	pthread_cleanup_pop(0);
	//------------------------------------------------------------------------------------------
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
