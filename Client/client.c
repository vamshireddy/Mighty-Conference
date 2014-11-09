#include "common_headers.h"
#include "socket_utilities.h"
#include "json_utilities.h"
#include "webserver.h"

pthread_mutex_t lock;

int sock_fd;

void* heart_beat_message(void* arg)
{
	int * tmp = (int *)arg;
	int sock_fd = *tmp;

	while(1)
	{
		printf("Sent BEEP\n");
		char* str = JSON_make_str("HEARTBEAT","BEEP");
		char* len_str = JSON_make_length_str(str);
		//lock
		pthread_mutex_lock(&lock);

		Write(sock_fd,len_str,JSON_LEN_SIZE);
		Write(sock_fd,str,strlen(str));

		//unlock
		pthread_mutex_unlock(&lock);
		sleep(4);
	}

}

main(){
	//invoke  the webserver
	ws_init();	

	if(pthread_mutex_init(&lock,NULL)!=0)
	{
		return 0;
	}

	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	
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

	printf("Enter your name : ");
	char name[USERNAME_LENGTH];
	scanf("%s",name);
	char str[40];
	sprintf(str,"{\"AUTH\":\"%s$not\"}",name);
	//char* str = "{\"AUTH\":\"$not\"}";
	int i;
	printf("Sending : %s\n\n",str);
	
	char* len_str = JSON_make_length_str(str);

	Write(sock_fd,len_str,JSON_LEN_SIZE);
	Write(sock_fd,str,strlen(str));

	// This should be after authentication
	pthread_t tid;
	if( pthread_create(&tid, NULL, heart_beat_message, (void*)&sock_fd) != 0)
	{
		printf("Failed to spawn a thread for the list monitor");
		return -1;
	}

	while(1)
	{
		char len_str1[JSON_LEN_SIZE+1];
		Read(sock_fd,len_str1,JSON_LEN_SIZE);
		len_str1[JSON_LEN_SIZE] = 0; // Making it a string

		int len = atoi(JSON_get_value_from_pair(len_str1, "LENGTH"));

		char strrecv[len+1];
		Read(sock_fd,strrecv,len);
		strrecv[len] = 0;	// This is for making the char array as a string
		printf("RECIEVED STRING :  %s\n\n",strrecv);
	}
	
	//closing the webserver
	ws_finalize();
}
