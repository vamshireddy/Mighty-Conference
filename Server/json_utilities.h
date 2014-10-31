#include "common_headers.h"
/*
	JSON Utility function for creating a length string
	Length string is nothing but, before transmitting a message, the client has to know how many char's should it 
	receive, so this length string will provide that to the client. Client then can use this length to receive the 
	actual string
*/
char* JSON_make_length_str(char* str)
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

/*
	This function will extract the value from the JSON {"key":"value"} pair
*/
char* JSON_get_value_from_pair(char* cli_strlen_JSON, char* key)
{
	// Extract the JSON part of the string
	json_error_t error; // For error, can be ignored
	json_t* root = json_loads(cli_strlen_JSON, 0, &error); // Make a JSON object of the received string

	if( root == NULL )
	{
		printf("Error is decoding JSON\n");
		exit(0);
	}

	// Get the value
	json_t* len_value_JSON = json_object_get(root, key); // Get the value JSON object of key

	// If not found, then return NULL
	if( len_value_JSON == NULL )
	{
		return NULL;
	}

	char* len_text = json_string_value(len_value_JSON); // Convert the value JSON object to string to get the length in string format

	return len_text;
}

/*
	Make JSON string with key value
*/

char* JSON_make_str(char* key,char* value)
{
	json_t* root = json_object();

	json_object_set_new(root,key,json_string(value));

	char* s = json_dumps(root, JSON_DECODE_ANY);

	return s;
}