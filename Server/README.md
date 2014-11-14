## Architecture 
### Connection Persistence
* Each client will be spawned a thread at the server for which a node is created, and also TCP socket.
* All the conenctions are persistent. Clients give Heartbeat messages to make the connection persistent.
* Server will try to proactively delete the connection if the last contacted time through the heartbeats is more than the specified time.
