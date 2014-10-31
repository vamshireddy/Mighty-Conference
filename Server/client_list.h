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
	Node format for the client's node in the online clients list.
*/
typedef struct client_node
{
	pthread_t tid; // Thread ID of the thread which controls the client communication job
	int clientfd;   // Socket of the communication
	char client_id[USERNAME_LENGTH]; // Client ID
	int reachable;
	struct sockaddr_in* client_addr;
	struct client_node* next;

}client_node_t;

/*
	Reader writer lock will lock the list when some writer thread tries to update, while providing 
	simultaneous reads to the reader threads, given there is no writer thread holding the writer lock.
*/
typedef struct clients_list
{
	// Head of the list
	client_node_t* head;
	// List reader-writer lock
	pthread_rwlock_t l_lock;
	// No of clients in the list
	int list_count;

}clients_list_t;

/*
	Global declaration of list
*/
clients_list_t* list = NULL;

/*
	Initalize the list
*/
int init_list()
{
	int err;
	list->head = NULL;
	list->list_count = 0;
	err = pthread_rwlock_init(&list->l_lock,NULL);

	if( err != 0 )
	{
		// Error in initializing the lock
		return err;
	}
	// Success
	printf("Read write lock initalized!\n");
	return 0;
}


/*
	This function will be called by the worker thread assigned for a client
*/

client_node_t* create_new_client(int s_id, struct sockaddr_in* cliaddr, char* userid)
{
	client_node_t* temp = (client_node_t*)malloc(sizeof(client_node_t));
	strcpy(temp->client_id, userid);
	temp->tid = pthread_self();
	temp->clientfd = s_id;
	temp->client_addr = cliaddr;
	temp->next = NULL;
	temp->reachable = 1;
	return temp;
}

/*
	Type : 0 --- deletion of client
	Type : 1 --- addition of client

	This informs everyone in the system about the addition of new client or deletion of old client
*/

int inform_everyone(char* client_id,int type)
{
	// 1. Create the JSON string 
	// 2. Find the length and make JSON string of it
	// 3. Inform the receiver about the length
	// 4. send the JSON string
	json_t* root = json_object();

	char type_str[11];
	if( type == 1 )
	{
		strcpy(type_str,"NEW_CLIENT");
	}
	else if( type == 0 )
	{
		strcpy(type_str,"DEL_CLIENT");
	}
	
	json_object_set_new(root,type_str,json_string(client_id));
	char* str_JSON = json_dumps(root, JSON_DECODE_ANY);

	// Send all the clients in the list
	client_node_t* temp = list->head;

	// Calculate the length and get the string, this will be sent to the client.
	char* len_str = JSON_make_length_str(str_JSON);
	printf("Length is %d and length string made is %s\n\n\n",strlen(str_JSON),len_str);

	while( temp!= NULL )
	{
		// Send the length to the client
		printf("\n\n\nI am sending %s-- to %s\n\n",len_str,temp->client_id);
		Write(temp->clientfd,len_str,strlen(len_str), temp);
		printf("Sent succesfully\n\n");
		// Now send the online clients string
		printf("\n\n\ni am sending %s-- to %s\n\n",str_JSON,temp->client_id);
		Write(temp->clientfd,str_JSON,strlen(str_JSON),temp);
		printf("Sent succesfully\n\n");
		temp = temp->next;
	}
	printf("----------SENT EVERYONE ABOUT THE UPDATE-------------\n");

}

/* 
	Add the client to the head of the list 
*/
void add_client(client_node_t* client, int* is_client_added)
{
	// Lock the list
	pthread_rwlock_wrlock(&list->l_lock);

	// Inform all other clients about this new client
	inform_everyone(client->client_id,1);

	list->list_count++;

	if( list->head == NULL )
	{
		list->head = client;
	}
	else
	{
		client_node_t* temp = list->head;
		list->head = client;
		client->next = temp;
	}
	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);
	// Client is added now!. Make it 1
	*is_client_added = 1;
}

/* Remove the client from the list */

void remove_client(pthread_t tid)
{

	// Lock the list
	pthread_rwlock_wrlock(&list->l_lock);

	// Client name holder
	char client_name[USERNAME_LENGTH];

	client_node_t* temp = list->head;
	client_node_t* prev = NULL;
	while( temp!= NULL )
	{

		if( pthread_equal(temp->tid,tid) != 0 )
		{
			// Found the node
			printf("\n\nFOUND THE NODE!!\n\n");
			if( prev == NULL )
			{
				list->head = temp->next;
			}
			else
			{
				prev->next = temp->next;
			}

			// Copy the name, so that we can use it
			strcpy(client_name,temp->client_id);
			free(temp);
			list->list_count--;
			break;
		}
		prev = temp;
		temp = temp->next;
	}
	printf("DONE WITH REMOVING\n");
	// INFORM EVERYONE ABOUT THE DELETION
	inform_everyone(client_name,0);

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);
}

/*
 	Displays the online clients in the system
 */
void display_clients()
{
	// Lock this list in read mode
	pthread_rwlock_rdlock(&list->l_lock);
	
	client_node_t* temp = list->head;

	printf("The clients are : \n");

	while( temp!= NULL )
	{
		printf("Client id : %s , Client thread id : %u \n",temp->client_id, (unsigned int)temp->tid);
		temp = temp->next;
	}

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);
}

/*
	List is traversed and JSON string is built 
*/

char* build_JSON_string_from_list(client_node_t* client)
{

	// Create JSON objects and populate them
	json_t* root,*cli_array;

	cli_array = json_array();

	// Lock this list in read mode
	pthread_rwlock_rdlock(&list->l_lock);

	client_node_t* temp = list->head;

	while( temp!= NULL )
	{
		// Skip sending its name to the it(client).
		if( strcmp(temp->client_id,client->client_id)==0 )
		{
			temp = temp->next;
			continue;
		}
	    	json_array_append_new(cli_array,json_string(temp->client_id));
		temp = temp->next;
	}

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);

	// Create a JSON object and add the client list to it
	root = json_object();
	json_object_set_new(root,"CLIENTS_LIST",cli_array);

	// Make a JSON string from the above object
	char* s = json_dumps(root, JSON_DECODE_ANY);

	printf("Built %s \n",s);

	return s;
}

