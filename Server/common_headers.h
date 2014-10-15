#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H
// ------------------START----------------------- //

#include <stdio.h>
#include <stdlib.h>


// pthreads

#include <pthread.h>

// Networking

#include <netinet/in.h>
#include <sys/socket.h>
#define SERV_PORT 4444
#define LISTENQ 20
#define INET_ADDRSTRLEN 16

// ------------------END-------------------------//

#endif /* COMMON_HEADERS_H */