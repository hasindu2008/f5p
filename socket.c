/* @socketLibC
**
** A simple and easy to use TCP Linux socket programming library
** https://github.com/hasindu2008/socketLibC
** @author: Hasindu Gamaarachchi (hasindu@unsw.edu.au)
** @@
******************************************************************************/
/*MIT License
Copyright (c) 2019 Hasindu Gamaarachchi
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "socket.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>

//verbosity
int verbose_socklibc = 1; //1 special-recommended //2 errors, //3 warnings //4 info

/*****************Internal Function prototypes that are not included in socket.h ***********************/

/*Die on error. Print the error and exit if the return value of the previous function is -1*/
void errorCheck(int ret, char* msg);

/*Send length number of bytes from the buffer. This is a blocking call.*/
void send_all(int socket, void* buffer, long length);

/*Receive length number of bytes to the buffer. This is a blocking call. Make sure that buffer is big enough.*/
void recv_all(int socket, void* buffer, long length);

//receive all but try only number of 'times' before giving up
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

	if(verbose_socklibc>=4){	
		INFO("Receiving a message of size %ld.", expected_length);
	}
	
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

	if(verbose_socklibc>=4){
		INFO("Receiving a message of size %ld.", expected_length);
	}
	
    if (expected_length > length && verbose_socklibc>=1) {
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
        0 && verbose_socklibc>=1) {
        WARNING("%s", "setsockopt(SO_REUSEADDR) failed");
    }

    //try binding
    ret = -1;
    while (ret == -1) {
        ret = bind(listenfd, (struct sockaddr*)&server, sizeof(server));
        if (ret == -1) {
            
			if(verbose_socklibc>=1){
				WARNING(
                "%s",
                "Cannot bind to address as it is already used. Trying again.");
			}
            sleep(1);
        }
    }

    /*** end of the sophisticated way*/

    /*following (commented) is the simplest way of doing it. But You will get "Cannot bind : Address is used"
    for few minutes after the program that used the port exited */
    //ret=bind(listenfd,(struct sockaddr *)&server, sizeof(server));

    errorCheck(ret, "Cannot bind");
    if(verbose_socklibc>=4){
		INFO("Binding successfull to port %d.", PORT);
	}

    //now listen
    ret = listen(listenfd, 5);
    errorCheck(ret, "Cannot listen");
	if(verbose_socklibc>=4){
		INFO("Listening on port %d.", PORT);
	}

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
	if(verbose_socklibc>=4){
		INFO("Client %s:%d connected.", ip, client.sin_port);
	}

    return connectfd;
}

/*Disconnect a connected client*/
void TCP_server_disconnect_client(int connectfd) {
    //close sockets
    int ret = close(connectfd);
    errorCheck(ret, "Cannot close socket.");
    if(verbose_socklibc>=4){
		INFO("%s", "Client disconnected.");
	}
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

    //tcpkeepalive activation (https://www.tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/)
    /* Set the option active */   
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    if(setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0 ) {
        if(verbose_socklibc>=1) {
			WARNING("%s","Could not enable tcp keepalive. Dead host detection will not work.");    
		}
    }
    else{
        //TCP_KEEPCNT: overrides tcp_keepalive_probes : the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
        //TCP_KEEPIDLE: overrides tcp_keepalive_time : the interval between the last data packet sent (simple ACKs are not considered data) and the first keepalive probe; after the connection is marked to need keepalive, this counter is not used any further
        //TCP_KEEPINTVL: overrides  tcp_keepalive_intvl : the interval between subsequential keepalive probes, regardless of what the connection has exchanged in the meantime
        optval=5;
        int ret1=setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPCNT, &optval, optlen);
        optval=300;
        int ret2=setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, optlen);
        optval=60;
        int ret3=setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, optlen);
        if(ret1<0 || ret2<0 || ret3<0){
            if(verbose_socklibc>=1){
				WARNING("%s","Could not set tcp keepalive parameters. Dead host detection may not work.");
			}
        }
                  
    }

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
    if(verbose_socklibc>=4){
		INFO("Connected to server %s.", ip);
	}

    return socketfd;
}


/* Connect to a TCP server at PORT at ip, but give up after 'times' number of times*/
int TCP_client_connect_try(char* ip, int PORT, int times) {
    //socket for connecting
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(socketfd, "Cannot create socket.");

    //tcp keepalive activation (https://www.tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/)
    /* Set the option active */   
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    if(setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        if(verbose_socklibc>=1) WARNING("%s","Could not enable tcp keepalive. Dead host detection will not work.");        
    }
    else{
        //TCP_KEEPCNT: overrides tcp_keepalive_probes : the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
        //TCP_KEEPIDLE: overrides tcp_keepalive_time : the interval between the last data packet sent (simple ACKs are not considered data) and the first keepalive probe; after the connection is marked to need keepalive, this counter is not used any further
        //TCP_KEEPINTVL: overrides  tcp_keepalive_intvl : the interval between subsequential keepalive probes, regardless of what the connection has exchanged in the meantime
        optval=5;
        int ret1=setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPCNT, &optval, optlen);
        optval=300;
        int ret2=setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, optlen);
        optval=60;
        int ret3=setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, optlen);
        if(ret1<0 || ret2<0 || ret3<0){
			if(verbose_socklibc>=1){
				WARNING("%s","Could not set tcp keepalive parameters. Dead host detection may not work.");
			}
        }
                  
    }

    //initializing the server address and assigning port numbers
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(PORT);

    //connecting. The following (comment) tries ones and giveup
    //int ret=connect(socketfd,(struct sockaddr *)&server, sizeof(server));

    //try connecting until successful. this is better than trying once and giving up
    int ret = -1;
    int counter = 0;
    while (ret == -1) {
        ret = connect(socketfd, (struct sockaddr*)&server, sizeof(server));
        counter++;
        if (counter >= times) {
            return -1;
        }
		if(verbose_socklibc>=4){	
			INFO("Connection attempt to %s failed. Trying again.", ip);		
		}
        sleep(1);
    }

    errorCheck(ret, "Cannot connect.");
    if(verbose_socklibc>=4){
		INFO("Connected to server %s.", ip);
	}

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
        if(verbose_socklibc>=1){
			perror(msg);
		}
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
            if(verbose_socklibc>=1) {
				WARNING("%s", "Could not send all data. Trying again.");
			}
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
			if(verbose_socklibc>=1){
				WARNING("%s", "Could not receive all data. Trying again.");
			}
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
			if(verbose_socklibc>=3){
				WARNING("%s", "Could not receive all data. Trying again.");
			}
            sleep(1);
            counter++;
            if (counter >= times) {
				if(verbose_socklibc>=2){
					ERROR("%s", "Socket is probably broken. Giving up.");
				}
                return -1;
            }
        }
        ptr += i;
        length -= i;
    }
    return 0;
}