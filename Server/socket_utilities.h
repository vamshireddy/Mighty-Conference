#include "common_headers.h"

int Socket(int domain, int type, int protocol)
{
	int ret = socket(domain,type,protocol);
	if( ret == -1 )
	{
		perror("Error in creating TCP socket!");
		exit(0);
	}
	return ret;
}

int Bind(int sockfd, const struct sockaddr* myaddr, socklen_t addrlen)
{
	int ret = bind(sockfd, myaddr, addrlen);
	if( ret == -1 )
	{
		perror("Error in binding the address at the server");
		exit(0);
	}
	return ret;
}

int Listen(int sockfd, int backlog)
{
	int ret;
	ret = listen(sockfd, backlog);
	if( ret == -1 )
	{
		perror("TCP server could not listen\n");
		exit(0);
	}
	return ret;
}

int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t* addrlen)
{
	int ret;

	ret = accept(sockfd, cliaddr, addrlen);

	if( ret == -1 )
	{
		perror("TCP server cannot accept incoming clients \n");
		exit(0);
	}
	return ret;
}