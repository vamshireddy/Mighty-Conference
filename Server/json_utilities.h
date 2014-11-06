#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H
#include "common_headers.h"
#endif

/*
	JSON Utility function for creating a length string. This will return an object which contains the string
	which can be fetched by the caller. ( Have to decrement the ref count after the string is extracted )
*/
json_t* JSON_make_length_str(char* str)
{
	int len = strlen(str);
	char len_str[LEN_STR_LENGTH];
	snprintf(len_str,6,"%5d",len);
	json_t* len_str_object = json_object();
	int err = json_object_set_new(len_str_object, "LENGTH", json_string(len_str));
	if( err == -1 )
	{
		printf("Error in json_object_get");
		exit(0);
	}
	return len_str_object;
}

/*
	This function will extract the value from the JSON {"key":"value"} pair
	If this function is used for extracting length from the length string, then atoi is necessary to the returned value of this
	function.
*/
json_t* JSON_get_value_from_pair(char* cli_strlen_JSON, char* key)
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
	// MOST IMPORTANT Line of this header file. I have spent two days figuring out this!
	// len_value_JSON will be a borrowed reference. Increment it so that we can work on this object
	// and later decrement reference
	// ALWAYS SHOULD CALL DECREF on objects which contain reference count>0
	json_incref(len_value_JSON);
	// If not found, then return NULL
	if( len_value_JSON == NULL )
	{
		return NULL;
	}
	return len_value_JSON;
}

/*
	Make JSON string with key and value
*/
json_t* JSON_make_str(char* key,char* value)
{
	json_t* root = json_object();
	json_object_set_new(root,key,json_string(value));
	return root;
}
