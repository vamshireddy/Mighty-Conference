#include "common_headers.h"

/*
	Node format for the client's node in the online clients list.
*/
typedef struct client_node
{
	pthread_t tid; // Thread ID of the thread which controls the client communication job
	int socket_id;   // Socket of the communication
	char client_id[USERNAME_LENGTH]; // Client ID
	struct sockaddr_in* client_addr;
	struct client_node* next;
}client_node_t;

/*
	Reader writer lock will lock the list when some writer thread tries to update, while providing 
	simultaneous reads to the reader threads when there is no writer thread holding the writer lock.
*/
typedef struct clients_list
{
	// Head of the list
	client_node_t* head;
	// List reader-writer lock
	int list_count;
	pthread_rwlock_t l_lock;

}clients_list_t;



int init_list(clients_list_t* list)
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

char* get_length_str(char* str)
{
	int len = strlen(str);
	char len_str[LEN_STR_LENGTH];
	snprintf(len_str,6,"%5d",len);

	json_t* len_str_object = json_object();
  	json_object_set_new(len_str_object, "length", json_string(len_str));


	char* s = json_dumps(len_str_object, JSON_DECODE_ANY);

	printf("Built length str :%sEND and length is %d\n",s,strlen(s));

	return s;
}

/*
	This function will be called by the worker thread assigned for a client */

client_node_t* create_new_client(int s_id, struct sockaddr_in* cliaddr)
{
	client_node_t* temp = (client_node_t*)malloc(sizeof(client_node_t));
	temp->tid = pthread_self();
	temp->socket_id = s_id;
	temp->client_addr = cliaddr;
	temp->next = NULL;
	return temp;
}


int inform_everyone(client_node_t* client,clients_list_t* list )
{
	// 1.Create the JSON string 
	// 2. Find the length and make JSON string of it
	// 3. Inform the receiver about the length
	// 4. send the JSON string
	json_t* root = json_object();
	json_object_set_new(root,"new_client",json_string(client->client_id));
	char* str_JSON = json_dumps(root, JSON_DECODE_ANY);

	// Send all the clients in the list

	client_node_t* temp = list->head;

	char* len_str = get_length_str(str_JSON);

	while( temp!= NULL )
	{
		// Send the length to the client
		Write(temp->socket_id,len_str,JSON_LEN_SIZE);
		// Now send the online clients
		Write(temp->socket_id,str_JSON,strlen(str_JSON)+1);
		printf("Written %sEND\n",str_JSON);
		temp = temp->next;
	}

	printf("SENT EVERYONE ABOUT THE UPDATE");

}

/* 
	Add the client to the head of the list 
*/

void add_client(clients_list_t* list, client_node_t* client)
{
	// Lock the list
	pthread_rwlock_wrlock(&list->l_lock);

	// Inform all other clients about this new client
	inform_everyone(client,list);

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
}

/* Remove the client from the list */

void remove_client(clients_list_t* list, pthread_t id)
{

	// Lock the list
	pthread_rwlock_wrlock(&list->l_lock);
	client_node_t* temp = list->head;
	client_node_t* prev = NULL;
	while( temp!= NULL )
	{

		if( pthread_equal(temp->tid,id) != 0 )
		{
			// Found the node
			if( prev == NULL )
			{
				list->head = temp->next;
				free(temp);
			}
			else
			{
				prev->next = temp->next;
				free(temp);
			}
			list->list_count--;
			break;
		}
		prev = temp;
		temp = temp->next;
	}
	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);
}

void display_clients(clients_list_t* list)
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

char* build_JSON_string_from_list(clients_list_t* list, client_node_t* client)
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
		json_t* tmp = json_object();
		json_object_set_new(tmp,"Client",json_string(temp->client_id));
	    json_array_append_new(cli_array,tmp);
		temp = temp->next;
	}

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);

	// Create a JSON object and add the client list to it
	root = json_object();
	json_object_set_new(root,"clients_list",cli_array);

	// Make a JSON string from the above object
	char* s = json_dumps(root, JSON_DECODE_ANY);

	printf("Built %s \n",s);

	return s;
}

