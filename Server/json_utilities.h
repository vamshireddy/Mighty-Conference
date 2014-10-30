#include "common_headers.h"
/*
	JSON Utility function for creating a length string
	Length string is nothing but, before transmitting a message, the client has to know how many char's should it 
	receive, so this length string will provide that to the client. Client then can use this length to receive the 
	actual string
*/
char* get_length_str(char* str)
{
	int len = strlen(str);
	char len_str[LEN_STR_LENGTH];
	snprintf(len_str,6,"%5d",len);
	json_t* len_str_object = json_object();
	json_object_set_new(len_str_object, "LENGTH", json_string(len_str));
	char* s = json_dumps(len_str_object, JSON_DECODE_ANY);
	printf("Built length str :%sEND and length is %d\n",s,strlen(s));
	return s;
}
