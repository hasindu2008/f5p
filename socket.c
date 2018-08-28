#include "socket.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*****************Internal Function prototypes that are not included in socket.h ***********************/

/*Die on error. Print the error and exit if the return value of the previous function is -1*/
void errorCheck(int ret, char* msg);

/*Send length number of bytes from the buffer. This is a blocking call.*/
void send_all(int socket, void* buffer, long length);

/*Receive length number of bytes to the buffer. This is a blocking call. Make sure that buffer is big enough.*/
void recv_all(int socket, void* buffer, long length);

//receive all but try olly times before giving up
int recv_all_try(int socket, void* buffer, long length, int times);

//some macros
#define WARNING(arg, ...)                                                      \
    fprintf(stderr, "[%s::WARNING]\033[1;33m " arg "\033[0m\n", __func__,      \
            __VA_ARGS__)
#define ERROR(arg, ...)                                                        \
    fprintf(stderr, "[%s::ERROR]\033[1;31m " arg "\033[0m\n", __func__,        \
            __VA_ARGS__)
#define INFO(arg, ...)                                                         \
    fprintf(stderr, "[%s::INFO] " arg "\n", __func__, __VA_ARGS__)

/*******************************Blocking send and receive***********************************/

/*Send the number of bytes to be sent. Then send the actual data.*/
void send_full_msg(int socket, void* buffer, long length) {
    send_all(socket, &length, sizeof(long));
    send_all(socket, buffer, length);
}

/*First receive the number of bytes to be received. Then receive the actual data to the buffer. */
long recv_full_msg(int socket, void* buffer, long length) {
    long expected_length = 0;
    recv_all(socket, &expected_length, sizeof(long));

    INFO("Receiving a message of size %ld.", expected_length);

    if (expected_length > length) {
        WARNING("Buffer of size %ld is too small to fit expected data.",
                expected_length);
    }

    recv_all(socket, buffer, expected_length);
    return expected_length;
}

long recv_full_msg_try(int socket, void* buffer, long length, int times) {
    long expected_length = 0;
    long ret = recv_all_try(socket, &expected_length, sizeof(long), times);
    if (ret < 0) {
        return ret;
    }

    INFO("Receiving a message of size %ld.", expected_length);

    if (expected_length > length) {
        WARNING("Buffer of size %ld is too small to fit expected data.",
                expected_length);
    }

    ret = recv_all_try(socket, buffer, expected_length, times);
    if (ret < 0) {
        return ret;
    }
    return expected_length;
}

/***************************Server side*******************************************************/

/*Create a TCP socket, bind and then listen*/
int TCP_server_init(int PORT) {
    //initialize variables
    int listenfd, ret;
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    //open socket bind and listen
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(listenfd, "Cannot create socket");

    /****sophisticated way of binding*/

    //This lets a port be used as soon as the using program ends. Otherwise you will get "Cannot bind : Address is used"
    //for few minutes after the program that previously used the port exists
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) <
        0) {
        WARNING("%s", "setsockopt(SO_REUSEADDR) failed");
    }

    //try binding
    ret = -1;
    while (ret == -1) {
        ret = bind(listenfd, (struct sockaddr*)&server, sizeof(server));
        if (ret == -1) {
            WARNING(
                "%s",
                "Cannot bind to address as it is already used. Trying agin.");
            sleep(1);
        }
    }

    /*** end of the sophisticated way*/

    /*following (commented) is the simplest way of doing it. But You will get "Cannot bind : Address is used"
    for few minutes after the program that used the port exited */
    //ret=bind(listenfd,(struct sockaddr *)&server, sizeof(server));

    errorCheck(ret, "Cannot bind");
    INFO("Binding successfull to port %d.", PORT);

    //now listen
    ret = listen(listenfd, 5);
    errorCheck(ret, "Cannot listen");
    INFO("Listening on port %d.", PORT);

    return listenfd;
}

/*Accept a client*/
int TCP_server_accept_client(int listenfd) {
    int connectfd;

    struct sockaddr_in client;
    socklen_t clientlen = sizeof(client);

    //accept connections
    connectfd = accept(listenfd, (struct sockaddr*)&client, &clientlen);
    errorCheck(connectfd, "Cannot accept");

    //get ip to string
    char ip[16];
    inet_ntop(AF_INET, &client.sin_addr, ip, clientlen);
    INFO("Client %s:%d connected.", ip, client.sin_port);

    return connectfd;
}

/*Disconnect a connected client*/
void TCP_server_disconnect_client(int connectfd) {
    //close sockets
    int ret = close(connectfd);
    errorCheck(ret, "Cannot close socket.");
    INFO("%s", "Client disconnected.");
}

/*Close down the listening socket*/
void TCP_server_shutdown(int listenfd) {
    //we do not need the listening socket now
    int ret = close(listenfd);
    errorCheck(ret, "Cannot close socket.");
}

/********************************Client side***************************************************/

/* Connect to a TCP server at PORT at ip*/
int TCP_client_connect(char* ip, int PORT) {
    //socket for connecting
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(socketfd, "Cannot create socket.");

    //initializing the server address and assigning port numbers
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(PORT);

    //connecting. The following (comment) tries ones and giveup
    //int ret=connect(socketfd,(struct sockaddr *)&server, sizeof(server));

    //try connecting until successful. this is better than trying once and giving up
    int ret = -1;
    while (ret == -1) {
        ret = connect(socketfd, (struct sockaddr*)&server, sizeof(server));
        sleep(1);
    }

    errorCheck(ret, "Cannot connect.");
    INFO("Connected to server %s.", ip);

    return socketfd;
}

/* Disconnect*/
void TCP_client_disconnect(int socketfd) {
    //closing sockets
    int ret = shutdown(socketfd, SHUT_RDWR);
    errorCheck(ret, "Cannot disconnect.");
    ret = close(socketfd);
    errorCheck(ret, "Cannot close socket.");
}

/************************************Internal Functions*****************************************/

/*Die on error. Print the error and exit if the return value of the previous function is -1*/
void errorCheck(int ret, char* msg) {
    if (ret == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

/*Send length number of bytes from the buffer. This is a blocking call.*/
void send_all(int socket, void* buffer, long length) {
    char* ptr = (char*)buffer;
    long i = 0;
    while (length > 0) {
        i = send(socket, ptr, length, 0);
        if (i < 1) {
            WARNING("%s", "Could not send all data. Trying again.");
            sleep(1);
        }
        ptr += i;
        length -= i;
    }
    return;
}

/*Receive length number of bytes to the buffer. This is a blocking call. Make sure that buffer is big enough.*/
void recv_all(int socket, void* buffer, long length) {
    char* ptr = (char*)buffer;
    long i = 0;
    while (length > 0) {
        i = recv(socket, ptr, length, 0);
        if (i < 1) {
            WARNING("%s", "Could not receive all data. Trying again.");
            sleep(1);
        }
        ptr += i;
        length -= i;
    }
    return;
}

/*Receive length number of bytes to the buffer. This is a blocking call. Make sure that buffer is big enough.*/
int recv_all_try(int socket, void* buffer, long length, int times) {
    char* ptr = (char*)buffer;
    long i = 0;
    int counter = 0;
    while (length > 0) {
        i = recv(socket, ptr, length, 0);
        if (i < 1) {
            WARNING("%s", "Could not receive all data. Trying again.");
            sleep(1);
            counter++;
            if (counter > times) {
                ERROR("%s", "Socket is probably broken. Giving up.");
                return -1;
            }
        }
        ptr += i;
        length -= i;
    }
    return 0;
}