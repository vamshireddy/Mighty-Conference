#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H
// ------------------START----------------------- //

#include <stdio.h>
#include <stdlib.h>
#define USERNAME_LENGTH 20
#define PASSWORD_LENGTH 20
#define LOGIN_STATUS_LENGTH 5


// Multithreading

#include <pthread.h>

// Networking

#include <netinet/in.h>
#include <sys/socket.h>
#define SERV_PORT 4444
#define LISTENQ 20
#define INET_ADDRSTRLEN 16

// ------------------END-------------------------//

#endif /* COMMON_HEADERS_H */