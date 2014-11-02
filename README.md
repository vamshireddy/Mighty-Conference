Mighty-Conference
=================

Mighty Conferencing Application is aimed to provide a way for professors to connect seamlessly and exchange their ideas in Computer Science Building at Manipal Institute Of Technology

#### Interaction between client and server ####


* Client and Server Agree on the format for the length string. It is defined in the json_utilities in the authentication_feature branch of the project. Its usually 16 bytes.

* Client can use this function and supply the string to be sent, it will calculate the length and make a json string and gives it to you, now the given json string is sent first( as it contains length ) and then your string


1. Client initiates a socket connection
2. Client gets the socket descriptor to connect to server
3. Client will send user name in this format

	{"AUTH":"akshay$password"} // To authenticate the client name and password 
								// Username and password have to be validated at the client side for checking whether they contain dollar '$'
4. If the client's details are valid
	
	{"AUTH_STATUS":"access"}

	If the client's details are invalid

	{"AUTH_STATUS":"deny"}

	Server send it back to the client.

5. Client will fetch the other online clients after it logs in, in the following format

	{"CLIENTS_LIST" : ["vamshi","akshay","nauti"] }

6. When someone logs in to the system, server sends the following message to all of the current clients

	{"NEW_CLIENT":"akshay"} 

   If someone goes out of the system, server sends the following message to all of the clients

    {"DEL_CLIENT":"vamshi"}

7. Heartbeack acks are received from the clients on the respective server threads

    {"HEARTBEAT":"beep"}

    {"HEARTBEAT":"beepbeep"}


#### How to send messages ####

* To send any message, length message should be constructed and sent to the other host. This message will have the number of chars that you are sending in the actual message.


* To send a string auth_str, we need to construct a length string called len_str. This function will do it.
char* len_str = JSON_make_length_str(auth_str);

* Then send the length string to the other node.
Write(clientfd, len_str, JSON_LEN_SIZE , *client);

* Now, send the actual string to the other node
Write(clientfd, auth_str, strlen(auth_str), *client);

#### How to receive messages ####

* Read the length string first   
Read(sock_fd,len_buff,JSON_LEN_SIZE);   

{"LENGTH":   34}   
 
* Now the length string is received, its time to get the length from the length string  
	int len = atoi(JSON_get_value_from_pair(len_buff, "LENGTH"));      
	---> 34

* Now use this and allocate the buffer   

char* buff = (char*)malloc(sizeof(len+1))   

* Now receive the actual string

Read(sock_fd,buff,len);

#### JSON Library (Jansson) ####

1. Install: 
+ https://jansson.readthedocs.org/en/2.7/gettingstarted.html#compiling-and-installing-jansson

2. Enter these commands 
+ ./configure
+ make
+ make check
+ make install

#### Installation ####

* Install the Jannson library as described above.
* Download the repo
+ run make in client folder
+ run make in server folder


#### Online Clients list - Synchronization Check Points ####
1. Clients connect to the server, server then add the new client to the Online clients List and spawns a new thread.

2. The data structure ( Online clients list ) is a linked list with all the connection nodes of every client.

3. Whenever someone is added to the list, it should be locked and be not allowed to access.

4. Each thread will traverse the linked list and prepare a packet for its client, which consists of all the other client names. When a thread is doing this, the data structure should be maintained consistent, nothing should be added or removed.

5. Each addition or removal, should trigger an event, which sends the update to the all other existing clients on the list.

6. If a client just fetched the online clients list, and someone goes out of the network, the thread of that terminated node, should remove its node from the online clients list and trigger an update.
