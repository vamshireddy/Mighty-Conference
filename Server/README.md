## Architecture 
### Thread Architecture
* The threads on the server are created per connection basis, on the go!
* Every client after just giving a hand shake, before logging in, a thread is created to handle its communication with the server.
* Threads are deleted if the clients are out of the system through the persistence timeout or client connection close.
* Monitor thread is created as soon as the server starts. It monitors the clients list and checks the persistence of them. If any client is found dead, then its task is to remove them from the system.
* Main thread will recieve the requests from the clients. On each successfull connection, a worker thread is created for each client.
### Connection Persistence
* Each client will be spawned a thread at the server for which a node is created, and also TCP socket.
* All the conenctions are persistent. Clients give Heartbeat messages to make the connection persistent.
* Server will try to proactively delete the connection if the last contacted time through the heartbeats is more than the specified time.
* Server gives three chances to the client before removing it from the system.
