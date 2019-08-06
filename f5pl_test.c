/* @f5pl_test.c
**
** test programme for fast5_pipeline
** @author: Hasindu Gamaarachchi (hasindu@unsw.edu.au)
** @@
******************************************************************************/
/*
MIT License
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
