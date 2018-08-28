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
#define MAX_PATH_SIZE 4096
//maximum limit for the number of tar files
#define MAX_FILES 4096
//maximum length of an IP address
#define MAX_IP_LEN 256
//maximum number of nodes (IP addresses)
#define MAX_IPS 256
//port which the f5p_daemon runs
#define PORT 20022
//maximum number of censecutive failues before retiring a node
#define MAX_CONSECUTIVE_FAILURES 3

//lock for accessing the list of tar files
pthread_mutex_t file_list_mutex = PTHREAD_MUTEX_INITIALIZER;

//the global data structure used by threads
typedef struct {
    char** file_list;      //the list of tar files
    int32_t file_list_idx; //the current index for file_list
    int32_t file_list_cnt; //the number of filled entires in file_list
    char** ip_list;        //the list of IP addresses
    int32_t failed
        [MAX_FILES]; //the indices (of file_list) for failed tar files (todo : malloc later)
    int32_t failed_cnt; //the number of failures
} node_global_args_t;

node_global_args_t core; //remember that core is used throughout

//thread function that handles each node
void* node_handler(void* arg) {
    //thread index
    int32_t tid = *((int32_t*)arg);

    //buffer for socket communication
    char buffer[MAX_PATH_SIZE];

    while (1) {
        pthread_mutex_lock(&file_list_mutex);
        int32_t fidx = core.file_list_idx;

        //check if there are files remaining to be processed
        if (fidx < core.file_list_cnt) {
            core.file_list_idx++;
            pthread_mutex_unlock(&file_list_mutex);
        } else {
            pthread_mutex_unlock(&file_list_mutex);
            break;
        }

        fprintf(stderr, "[t%d(%s)::INFO] Connecting to %s.\n", tid,
                core.ip_list[tid], core.ip_list[tid]);
        int socketfd = TCP_client_connect(core.ip_list[tid], PORT);

        fprintf(stderr, "[t%d()%s::INFO] Assigning %s to %s.\n", tid,
                core.ip_list[tid], core.file_list[fidx], core.ip_list[tid]);
        send_full_msg(socketfd, core.file_list[fidx],
                      strlen(core.file_list[fidx]));

        //receive the ''done' message (will block until done)
        int received = recv_full_msg_try(socketfd, buffer, MAX_PATH_SIZE, 5);

        //handle incase the socket has broken
        int32_t count = 0;
        while (received < 0) {
            count++;
            if (count >= MAX_CONSECUTIVE_FAILURES) {
                fprintf(stderr,
                        "[t%d(%s)::ERROR]\033[1;31m Device %s failed %d times "
                        "consecutively. Retiring the device. \033[0m\n",
                        tid, core.ip_list[tid], core.ip_list[tid], count);
                pthread_mutex_lock(&file_list_mutex);
                int32_t failed_cnt = core.failed_cnt;
                core.failed_cnt++;
                core.failed[failed_cnt] = fidx;
                pthread_mutex_unlock(&file_list_mutex);
                pthread_exit(0);
            }
            fprintf(stderr, "[t%d(%s)::INFO] Connecting to %s\n", tid,
                    core.ip_list[tid], core.ip_list[tid]);
            socketfd = TCP_client_connect(core.ip_list[tid], PORT);
            fprintf(stderr, "[t%d()%s::INFO] Assigning %s to %s\n", tid,
                    core.ip_list[tid], core.file_list[fidx], core.ip_list[tid]);
            send_full_msg(socketfd, core.file_list[fidx],
                          strlen(core.file_list[fidx]));
            received = recv_full_msg_try(socketfd, buffer, MAX_PATH_SIZE, 5);
        }

        //print the message
        buffer[received] = '\0'; //null character before printing the string
        fprintf(stderr, "[t%d(%s)::INFO] Recieved message '%s'.\n", tid, core.ip_list[tid],
                buffer);

        //close the connection
        TCP_client_disconnect(socketfd);
    }

    pthread_exit(0);
}

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

    //argument check check
    if (argc != 3) {
        ERROR("Not enough arguments. Usage %s <ip_list> <file_list>\n",
              argv[0]);
        exit(EXIT_FAILURE);
    }

    //read the list of tar files
    char** file_list = (char**)malloc(sizeof(char*) * (MAX_FILES));
    MALLOC_CHK(file_list);
    int32_t file_list_cnt = 0;
    int32_t i = 0;
    char* file_list_name = argv[2];
    FILE* file_list_fp = fopen(file_list_name, "r");
    NULL_CHK(file_list_fp);
    while (1) {
        size_t line_size = MAX_PATH_SIZE;
        char* line =
            malloc(sizeof(char) * (line_size)); //filepath+newline+nullcharacter
        MALLOC_CHK(line);
        int32_t readlinebytes = getline(&line, &line_size, file_list_fp);
        if (readlinebytes == -1) { //file has ended
            free(line);
            break;
        } else if (readlinebytes > MAX_PATH_SIZE) {
            free(line);
            ERROR("The file path %s is longer hard coded limit %d\n", line,
                  MAX_PATH_SIZE);
            exit(EXIT_FAILURE);
        }
        if (file_list_cnt > MAX_FILES) {
            free(line);
            ERROR("The number of entries in %s exceeded the hard coded limit "
                  "%d\n",
                  file_list_name, MAX_FILES);
            exit(EXIT_FAILURE);
        }
        if (line[readlinebytes - 1] == '\n' ||
            line[readlinebytes - 1] == '\r') {
            line[readlinebytes - 1] = '\0';
        }
        file_list[file_list_cnt] = line;
        file_list_cnt++;
    }
    fclose(file_list_fp);

    //read the list of nodes
    char** ip_list = (char**)malloc(sizeof(char*) * (MAX_IPS));
    MALLOC_CHK(ip_list);
    char* ip_list_name = argv[1];
    FILE* ip_list_fp = fopen(ip_list_name, "r");
    NULL_CHK(ip_list_fp);
    int32_t ip_cnt = 0;
    while (1) {
        size_t line_size = MAX_IP_LEN;
        char* line =
            malloc(sizeof(char) * (line_size)); //ip+newline+nullcharacter
        MALLOC_CHK(line);
        int32_t readlinebytes = getline(&line, &line_size, ip_list_fp);
        if (readlinebytes == -1) { //file has ended
            free(line);
            break;
        } else if (readlinebytes > MAX_IP_LEN) {
            free(line);
            ERROR("The IP length %s is longer hard coded limit %d\n", line,
                  MAX_IP_LEN);
            exit(EXIT_FAILURE);
        }
        if (ip_cnt > MAX_IPS) {
            free(line);
            ERROR("The number of entries in %s exceeded the hard coded limit "
                  "%d\n",
                  ip_list_name, MAX_IPS);
            exit(EXIT_FAILURE);
        }
        if (line[readlinebytes - 1] == '\n' ||
            line[readlinebytes - 1] == '\r') {
            line[readlinebytes - 1] = '\0';
        }
        ip_list[ip_cnt] = line;
        ip_cnt++;
    }
    fclose(ip_list_fp);

    //data structures for threads
    pthread_t node_thread[MAX_IPS] = {0};
    int32_t thread_id[MAX_IPS] = {0};

    core.file_list = file_list;
    core.file_list_cnt = file_list_cnt;
    core.file_list_idx = 0;
    core.ip_list = ip_list;
    core.failed_cnt = 0;

    //create threads for each node
    for (i = 0; i < ip_cnt; i++) {
        thread_id[i] = i;
        int ret = pthread_create(&node_thread[i], NULL, node_handler,
                                 (void*)(&thread_id[i]));
        if (ret != 0) {
            ERROR("Error joining thread %d", i);
            exit(EXIT_FAILURE);
        }
    }

    //joining client side threads
    for (i = 0; i < ip_cnt; i++) {
        int ret = pthread_join(node_thread[i], NULL);
        if (ret != 0) {
            ERROR("Error joining thread %d", i);
            //exit(EXIT_FAILURE);
        }
    }

    //failed files ones
    for (i = 0; i < core.failed_cnt; i++) {
        int id = core.failed[i];
        ERROR("%s was skipped due to a device retire", core.file_list[id]);
    }

    //todo : free iplist and filelist

    return 0;
}
