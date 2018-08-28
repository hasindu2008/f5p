#include "error.h"
#include "f5pmisc.h"
#include "socket.h"
#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//maximum limit for a file path
#define PATH_MAX 4096
//upper limit for the communication buffer
#define BUFFER_SIZE 4096
//hardcoded location of the pipeline script
#define SCRIPT "/nanopore/bin/fast5_pipeline.sh"
//port in which the deamon will listen
#define PORT 20022

void sig_handler(int sig) {
    void* array[100];
    size_t size = backtrace(array, 100);
    ERROR("I regret to inform that a segmentation fault occurred. But at least "
          "it is better than a wrong answer%s",
          ".");
    fprintf(stderr,
            "[%s::DEBUG]\033[1;35m Here is the backtrace in case it is of any "
            "use:\n",
            __func__);
    backtrace_symbols_fd(&array[2], size - 1, STDERR_FILENO);
    fprintf(stderr, "\033[0m\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    signal(SIGSEGV, sig_handler);

    //buffer for communication
    char buffer[BUFFER_SIZE];

    //create a listening socket on port
    int listenfd = TCP_server_init(PORT);

    while (1) {
        //accept a client connection
        int connectfd = TCP_server_accept_client(listenfd);

        //get message from client
        int received = recv_full_msg(connectfd, buffer, BUFFER_SIZE);

        //print the message
        buffer[received] = '\0'; //null character before printing the string
        INFO("Recieved %s.", buffer);

        //some hidden quit method
        if (strcmp(buffer, "quit!") == 0) {
            return 0;
        }

        //execute the script
        char command[PATH_MAX * 2];
        sprintf(command, "%s %s", SCRIPT, buffer);
        INFO("Command to be run %s.", command);
        int pid = system_async(command);
        wait_async(pid);

        //Copy a string to buffer
        strcpy(buffer, "done.");

        //send a message to the client
        send_full_msg(connectfd, buffer, strlen(buffer));

        //close down the client connection
        TCP_server_disconnect_client(connectfd);
    }

    //close the down the listening socket
    TCP_server_shutdown(listenfd);

    return 0;
}
