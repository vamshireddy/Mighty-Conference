#include <stdio.h>
#include <netinet/in.h>

int main()
{
	int sock_fd = socket(AF_INET,SOCK_STREAM,0);
	
	// Create the server socket
	struct sockaddr_in server_sock;
	// Initialize the socket structure to zero
	bzero(&server_sock,sizeof(struct sockaddr_in));
	printf("Enter the port number of server \n");
	int port;
	scanf("%d",&port);
	server_sock.sin_port = htons(port);
	server_sock.sin_family = AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&server_sock.sin_addr);

	connect(sock_fd,(struct sockaddr*)&server_sock,sizeof(struct sockaddr_in));

	char username[20];
	scanf("%s",username);

	printf("The string is\n");
	int i;
	for(i=0;i<20;i++)
	{
		printf("%c ",username[i]);
	}
	printf("Now sending\n");

	write(sock_fd,username,20);

	scanf("%s",username);

	printf("The string is\n");
	for(i=0;i<20;i++)
	{
		printf("%c ",username[i]);
	}
	printf("Now sending\n");

	write(sock_fd,username,20);

}
