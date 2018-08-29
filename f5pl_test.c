#include "error.h"
#include "f5pmisc.h"
#include "socket.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 4096
#define PORT 20022

int main(int argc, char* argv[]) {
    //argument check check
    if (argc != 3) {
        ERROR("Not enough arguments. Usage  %s <ip_address> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFF_SIZE];

    char* filepath = argv[2];
    int length = strlen(filepath);
    if (length > BUFF_SIZE) {
        ERROR("The file path is longer than the maximum allowed size %d\n",
              BUFF_SIZE);
        exit(EXIT_FAILURE);
    }

    //Connect to the server given as argument on port
    int socketfd = TCP_client_connect(argv[1], PORT);

    //send the message
    send_full_msg(socketfd, filepath, length);

    //receive the message
    int received = recv_full_msg(socketfd, buffer, BUFF_SIZE);

    //print the message
    buffer[received] = '\0'; //null character before printing the string
    fprintf(stderr, "Recieved : %s\n", buffer);

    //close the connection
    TCP_client_disconnect(socketfd);

    return 0;
}
