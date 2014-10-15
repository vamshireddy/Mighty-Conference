#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include "common_headers.h"

typedef struct argument_list
{
	int clientfd;
	struct sockaddr_in* cliaddr;
}args_list_t;

void* client_init_function(void* a)
{
	// New thread
	args_list_t* args = (args_list_t*)a;

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, (args->cliaddr)->sin_addr, ip, INET_ADDRSTRLEN);

	printf("New thread (%u) created for the client ( %d ) and IP address = %s\n", (unsigned int)pthread_self(), args->clientfd, ip);

}

int handle_client_request(int clientfd, const struct sockaddr_in cliaddr)
{
	// Create a new thread for this client and return this function

	// Build the arguments structure
	args_list_t* args = (args_list_t*)malloc(sizeof(args_list_t));
	args->clientfd = clientfd;
	args->cliaddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	memcpy(args->cliaddr, &cliaddr, sizeof(cliaddr));

	pthread_t tid;
	if( pthread_create(&tid, NULL, client_init_function, (void*)args) != 0)
	{
		printf("Failed to spawn a thread for the client");
		return -1;
	}
}


#endif /* COMMON_HEADERS_H */