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
	pthread_t tid;  // Thread ID of the thread which controls the client communication job
	int clientfd;   // Socket of the client
	char client_id[USERNAME_LENGTH]; // Client ID
	int attempt_count;	// If this becomes >=3 then the node is deleted
	time_t last_contacted_time;
	struct sockaddr_in* client_addr;  // Client IP and Port number
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
	Global declaration of list.
	Can be accessed by any thread.
	It will be initialized by main thread.
*/
clients_list_t* list = NULL;


/*
	Monitor the list and remove stale clients
*/
void* monitor_list()
{

	/*
		This is a seperate thread which is spawned by the main function for the purpose
		of removing dead clients.
		This has same lifetime as the main thread.
		This will kill other dead client threads.
	*/
	printf("Started monitoring!!!!!\n");
	pthread_t dead_clients[MAX_DEAD_CLIENTS];

	int count = 1;

	while( 1 )
	{
		printf("Dead client collection ride %d\n",count);
		int dead_client_count = 0;
		// Lock the list
		pthread_rwlock_rdlock(&list->l_lock);
		client_node_t* temp = list->head;
		time_t cur_time;
		time(&cur_time);
		printf("Acquired lock\n");
		while(temp!=NULL)
		{
			printf("Now looping!");
			/*
				Loop through the current list and check the difference between last received time
				and current time.
			*/
			printf ("OLD %s", ctime (&temp->last_contacted_time));
			printf ("NEW %s", ctime (&cur_time));
			double diff_time = difftime(cur_time,temp->last_contacted_time);
			printf("DIFF TIME is %f\n",diff_time);
			if( diff_time > 5 ) // diff > 5 seconds
			{
				// Atempts increased by 1
				temp->attempt_count++;
				printf("Attempt for %s is %d\n",temp->client_id,temp->attempt_count);
			}
			else
			{
				temp->attempt_count = 0;
			}
			// Check it the attempts >= 3, if it is then add the threadid to the deadclients
			// list to kill it.
			if( temp->attempt_count > 3 )
			{
				dead_clients[dead_client_count++] = temp->tid;
			}
			temp = temp->next;
		}
		// Unlock the list
		pthread_rwlock_unlock(&list->l_lock);
		/*
			Now start killing the dead clients ( remove nodes )
		*/
		int i;
		for(i=0;i<dead_client_count;i++)
		{
			printf("Killing %d\n",dead_clients[i]);
			// Remove the client node.
			remove_client(dead_clients[i]);
			printf("Removed node\n");
			// Kill the client thread.
			if(pthread_cancel(dead_clients[i]) == 0)
			{
				printf("THREAD CANCELLED PROPERLY\n");
			}
			else
			{
				printf("THREAD DIDNT CANCELLED PROPERLY\n");
			}

		}
		// Now all the dead clients are killed.
		// Sleep for 10 seconds and continue again.
		sleep(10);
		count++;
	}
}

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
	temp->attempt_count = 0;
    time(&temp->last_contacted_time); // store the current time
	return temp;
}


/*
	Type : 0 --- deletion of client
	Type : 1 --- addition of client

	This informs everyone in the system about the addition of new client or deletion of old client
	DONT USE LOCK FOR THIS!!! LOCK IS BEING HANDLED BY THE CALLER OF THIS FUNCTION
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

	// Calculate the length and get the string, this will be sent to the client.
	json_t* json_obj = JSON_make_length_str(str_JSON);
	// Extract the string from the JSON object
	char* len_str = json_dumps(json_obj, JSON_DECODE_ANY);
	//printf("Length is %d and length string made is %s\n\n\n",strlen(str_JSON),len_str);

	// Send all the clients in the list
	client_node_t* temp = list->head;

	while( temp!= NULL )
	{
		// Send the length to the client
		//printf("\n\n\nI am sending %s-- to %s\n\n",len_str,temp->client_id);
		Write(temp->clientfd,len_str,strlen(len_str), temp);
		//printf("Sent succesfully\n\n");
		// Now send the online clients string
		//printf("\n\n\ni am sending %s-- to %s\n\n",str_JSON,temp->client_id);
		Write(temp->clientfd,str_JSON,strlen(str_JSON),temp);
		//printf("Sent succesfully\n\n");
		temp = temp->next;
	}
	printf("----------SENT EVERYONE ABOUT THE UPDATE-------------\n");
	// Free up the JSON objects
	json_decref(json_obj);
	json_decref(root);

}

/* 
	Add the client to the head of the list 
*/
void add_client(client_node_t* client, int* is_client_added)
{
	// Lock the list
	printf("before write lock in add_client\n");
	pthread_rwlock_wrlock(&list->l_lock);
	printf("after write lock in add_client\n");

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
	printf("Unlocked the lock\n");
	// Client is added now!. Make it 1
	*is_client_added = 1;
}

/* Remove the client from the list */

void remove_client(pthread_t tid)
{

	// Lock the list
	printf("before write lock in rem_client\n");
	pthread_rwlock_wrlock(&list->l_lock);
	printf("after write lock in rem_client\n");
	// Client name holder
	char client_name[USERNAME_LENGTH];

	client_node_t* temp = list->head;
	client_node_t* prev = NULL;
	while( temp!= NULL )
	{

		if( pthread_equal(temp->tid,tid) != 0 )
		{
			// Found the node
			//printf("\n\nFOUND THE NODE!!\n\n");
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
	// printf("DONE WITH REMOVING\n");
	// INFORM EVERYONE ABOUT THE DELETION
	inform_everyone(client_name,0);

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);
}

/*
	Check if the clientId is currentlt in the list or not
	returns 1 --> if the clientID is present in the list
	return 0 --> if not
*/

int check_client(char* clientID)
{
	// Lock this list in read mode
	pthread_rwlock_rdlock(&list->l_lock);

	client_node_t* temp = list->head;

	int flag = 0;

	while( temp!= NULL )
	{
		if( strcmp(temp->client_id,clientID) == 0 )
		{
			// Already present, return -1
			// Unlock the list
			flag = 1;
			break;
		}
		temp = temp->next;
	}

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);

	return flag;
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
		printf("Client id : %s , Client thread id : %u "
			", last_contacted_time = %s, attempt_count = %d\n",temp->client_id, (unsigned int)temp->tid, ctime(&temp->last_contacted_time), temp->attempt_count);
		temp = temp->next;
	}

	// Unlock the list
	pthread_rwlock_unlock(&list->l_lock);
}

/*
	List is traversed and JSON string is built 
*/

json_t* build_JSON_string_from_list(client_node_t* client)
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
	printf("Unlocked the lock");

	// Create a JSON object and add the client list to it
	root = json_object();
	json_object_set_new(root,"CLIENTS_LIST",cli_array);
	return root;
}

