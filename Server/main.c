#include "client_list.h"
#include "socket_utilities.h"
#include "handle_client.h"
#include "common_headers.h"


pthread_t tid;

void* input_commands_function(void* arg)
{
	clients_list_t* list = (clients_list_t*)arg;

	int a;
	printf("Press 1 to view the online clients\n\n");
	while(1)
	{
		scanf("%d",&a);
		if( a == 1 )
		{
			display_clients(list);
		}
	}

}

int main()
{	
	printf("Started Application................\n");
	/*
		Start listening for client connections 
	*/
	clients_list_t* list = (clients_list_t*)malloc(sizeof(clients_list_t));

	/*
	    Initialize the list 
	*/
	init_list(list);

	/*
		Listen to the requests. if any request from the client - spawn a thread, add the client to the list
	*/

	int listenfd, connfd;

	socklen_t clilen;

	struct sockaddr_in cliaddr,servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	// Clear the server structure
	bzero(&servaddr, sizeof(servaddr));
	bzero(&cliaddr, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	Listen(listenfd,LISTENQ);

	// Before accespting clients, create a thread for handling user commands to query clients,etc 

	if( pthread_create(&tid, NULL, input_commands_function, (void*)list) != 0)
	{
		printf("Failed to spawn a thread for the client");
		return -1;
	}

	for(;;)
	{
		// Accept the clients and spawn a new thread for each of them
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
		handle_client_request(list, connfd, cliaddr);
	}

}
