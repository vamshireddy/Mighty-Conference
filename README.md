Mighty-Conference
=================

Mighty Conferencing Application is aimed to provide a way for professors to connect seamlessly and exchange their ideas in Computer Science Building at Manipal Institute Of Technology


#### Online Clients list - Synchronization Check Points ####
1. Clients connect to the server, server then add the new client to the Online clients List and spawns a new thread.

2. The data structure ( Online clients list ) is a linked list with all the connection nodes of every client.

3. Whenever someone is added to the list, it should be locked and be not allowed to access.

4. Each thread will traverse the linked list and prepare a packet for its client, which consists of all the other client names. When a thread is doing this, the data structure should be maintained consistent, nothing should be added or removed.

5. Each addition or removal, should trigger an event, which sends the update to the all other existing clients on the list.

6. If a client just fetched the online clients list, and someone goes out of the network, the thread of that terminated node, should remove its node from the online clients list and trigger an update.
