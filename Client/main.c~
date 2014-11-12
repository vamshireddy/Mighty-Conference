#include "common_headers.h"
#include "socket_utilities.h"
#include "json_utilities.h"
#include "webserver.h"
#include "client.h"

pthread_mutex_t lock;

extern int sock_fd;

main(){
	//invoke the webserver
	ws_init();	
	//invoke the socket client
	client_init();	//this is blocking
	
	while(1){
		char len_str1[JSON_LEN_SIZE+1];
		//getting data length
		Read(sock_fd,len_str1,JSON_LEN_SIZE);
		len_str1[JSON_LEN_SIZE] = 0; // Making it a null terminated string

		//extract length from string
		int len = atoi(JSON_get_value_from_pair(len_str1, "LENGTH"));
		
		char strrecv[len+1];
		//reading `len` bytes of data 
		Read(sock_fd,strrecv,len);
		strrecv[len] = 0;	// This is for making the char array as a string
		//need to send data to parse
		client_parse(strrev);
	}
	//closing the webserver
	ws_finalize();
}
