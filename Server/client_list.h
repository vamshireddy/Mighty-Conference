#include "common_headers.h"

/*
	Node format for the client's node in the online clients list.
*/
typedef struct client_node
{
	pthread_t tid; // Thread ID of the thread which controls the client communication job
	int socket_id;   // Socket of the communication
	int client_id; // Client ID
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

/*
	This function will be called by the worker thread assigned for a client */

client_node_t* create_new_client(int s_id, int id, struct sockaddr_in* cliaddr)
{
	client_node_t* temp = (client_node_t*)malloc(sizeof(client_node_t));
	temp->tid = pthread_self();
	temp->socket_id = s_id;
	temp->client_id = id;
	memcpy(temp->client_addr, cliaddr, sizeof(cliaddr));
	temp->next = NULL;
	return temp;
}

/* 
	Add the client to the head of the list 
*/

void add_client(clients_list_t* list, client_node_t* client)
{
	// Lock the list
	pthread_rwlock_wrlock(&list->l_lock);

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

void remove_client(clients_list_t* list, int id)
{

	// Lock the list
	pthread_rwlock_wrlock(&list->l_lock);
	client_node_t* temp = list->head;
	client_node_t* prev = NULL;
	while( temp!= NULL )
	{

		if( temp->client_id == id )
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
	client_node_t* temp = list->head;

	printf("The clients are : \n");

	while( temp!= NULL )
	{
		printf("Client id : %d , Client thread id : %u \n",temp->client_id, (unsigned int)temp->tid);
		temp = temp->next;
	}
}
