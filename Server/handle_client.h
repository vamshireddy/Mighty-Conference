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

int delete_client_thread(int is_client_added_to_list)
{
	// Now deletion of the client has two parts.
	// 1) If the client has not yet logged in ,then his node won't be present in the list, so we can directly delete
	// the thread by pthread_exit()

	// 2) If the client has terminated after logging in, then we need to remove the node from the list and send a
	// Notification to all other clients/


	// If the client's node is added in the list, then we have to delete it and send notifications to
	// all the other clients about the deletion, so that they can refresh their online clients list.
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

void send_length_of_msg(int clientfd, char* str, int is_client_added_to_list)
{
	int len = strlen(str);
	char len_str[LEN_STR_LENGTH];
	snprintf(len_str,6,"%5d",len);

	json_t* len_str_object = json_object();
  	json_object_set_new(len_str_object, "length", json_string(len_str));


	char* s = json_dumps(len_str_object, JSON_DECODE_ANY);

	printf("Built length str :%sEND and length is %d\n",s,strlen(s));

	// Send the length to the client
	Write(clientfd,s,JSON_LEN_SIZE,is_client_added_to_list);
}

/*
	Send the list of the clients who are logged in currently to client with clientfd.
*/
int send_clients_list(client_node_t* client, int is_client_added_to_list)
{
	char* string = build_JSON_string_from_list(client);
	
	// Send the length first
	send_length_of_msg(client->clientfd, string, is_client_added_to_list);

	// Now send the online clients
	Write(client->clientfd,string,strlen(string)+1,is_client_added_to_list);
	printf("Written %sEND\n",string);
}

int Read(int clientfd, char* buffer, int size, int is_client_added_to_list)
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
		delete_client_thread(is_client_added_to_list);
		return 0;
	}
}

int Write(int clientfd, char* buff, int len, int is_client_added_to_list)
{
	int left_chars = len;
	int written_chars = 0;

	int temp;
	while( left_chars > 0 )
	{
		if( (temp = write(clientfd, buff+written_chars, left_chars)) <= 0 )
		{
			// Error
			if( temp < 0 && errno == EINTR)
			{
				continue;
			}
			else
			{
				perror("Error with writing\n");
				return -1;
			}
		}
		written_chars += temp;
		left_chars -= left_chars;
	}
}

void* client_function(void* a)
{
	// New thread
	args_list_t* args = (args_list_t*)a;

	// -------Arguments ---------//
	clients_list_t* list = args->list;
	int clientfd = args->clientfd;	  // Client socket descriptor
	int is_client_added_to_list = 0; // if the client is added to the list, then this flag will be set
	// --------------------------//

	printf("New thread (%u) created for the client (%d)\n", (unsigned int)pthread_self(), clientfd);

	// Read the username of 20 characters and password of 20 characters and authenticate the client

	// Read the username
	printf("Client is entering username : ");
	char username[USERNAME_LENGTH];
	Read(clientfd, username, USERNAME_LENGTH, is_client_added_to_list);
	printf("\nThe username is %s\n",username);

	// Read the password
	char password[PASSWORD_LENGTH];
	Read(clientfd, password, PASSWORD_LENGTH, is_client_added_to_list);
	printf("The password is %s\n",password);


	// Validate the username and password
	while( authenticate(username,password) == 0 )
	{	
		// Status can be OKAY or DENY

		// Deny access as the credentials are invalid
		char* status = "DENY";
		printf("Client %s is trying to log in, the credentials are invalid!\n",username);
		Write(clientfd, status, LOGIN_STATUS_LENGTH, is_client_added_to_list);

		// Read username again
		Read(clientfd, username, USERNAME_LENGTH, is_client_added_to_list);
		printf("The username is %s\n",username);

		// Read password again
		Read(clientfd, password, PASSWORD_LENGTH, is_client_added_to_list);
		printf("The password is %s\n",password);
	}
	
	// Logged in succesfully
	// Now its time to create his entry in the clients list


	// Create a client node and 
	client_node_t* client = create_new_client(args->clientfd, args->cliaddr);

	printf("New client created\n");

	// Copy the user name to the client node on the list
	strncpy(client->client_id,username,USERNAME_LENGTH);

	// Add the client to the list
	add_client(client, &is_client_added_to_list);

	printf("Client node created succesfully");
	printf("Client %s logged in succesfully\n",client->client_id);

	// Send the status back to the client to make him access the application
	char* status = "OKAY";
	Write(clientfd, status, LOGIN_STATUS_LENGTH, is_client_added_to_list);

	// Now send all online clients in the list to the client
	send_clients_list(client,is_client_added_to_list);

}

int handle_client_request(int clientfd, const struct sockaddr_in cliaddr)
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
