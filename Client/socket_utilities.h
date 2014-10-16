#include "common_headers.h"


int Write(int sockfd, char* buff, int len)
{
	if( write(sockfd,buff,len) < len)
	{
		perror("Error in writing\n");
		exit(0);
	}
}

int Read(int servfd, char* buffer, int size)
{
	int char_count = size;
	int chars_read = 0;
	while( ( chars_read = read(servfd, buffer + chars_read , char_count ) ) > 0 )
	{
		char_count = char_count - chars_read;
		if( char_count == 0 )
		{
			// All chars are read, break out
			break;
		}
	}

	if( chars_read == -1 )
	{
		perror("Error in reading the line in readLine function : handle_client.h\n");
		exit(0);
	}
	else if( chars_read == 0 )
	{
		printf("Client's connection is terminated\n");
		exit(0);
	}
}

int read_line(int servfd, char* buffer)
{
	int char_count = size;
	int chars_read = 0;
	while( ( chars_read = read(servfd, buffer + chars_read , char_count ) ) > 0 )
	{
		char_count = char_count - chars_read;
		if( char_count == 0 )
		{
			// All chars are read, break out
			break;
		}
	}
}