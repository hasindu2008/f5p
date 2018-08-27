#include "error.h"
#include "f5pmisc.h"
#include "socket.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_SIZE 4096
#define MAX_FILES 4096

#define MAX_IP_LEN	256
#define MAX_IPS	256

#define PORT 20022


pthread_mutex_t file_list_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	char** file_list;
	int32_t file_list_idx;
	int32_t file_list_cnt;
	char** ip_list;
}node_global_args_t;

node_global_args_t node_global_args;

void *node_handler(void *arg){
	
	int32_t id = *((int32_t *)arg);
	
	char buffer[MAX_PATH_SIZE];
	
	while(1){
		

		pthread_mutex_lock(&file_list_mutex);
		int32_t i =  node_global_args.file_list_idx;
		if (i<node_global_args.file_list_cnt){
			node_global_args.file_list_idx++;
			pthread_mutex_unlock(&file_list_mutex);
		}
		else{
			pthread_mutex_unlock(&file_list_mutex);
			break;
		}
		
		//Connect to the server given as argument on port
		fprintf(stderr, "Connecting to %s\n", node_global_args.ip_list[id]);
		int socketfd = TCP_client_connect(node_global_args.ip_list[id], PORT);		
		
		fprintf(stderr, "Assign %s to %s\n", node_global_args.file_list[i],node_global_args.ip_list[id]);
		//send the message
		send_full_msg(socketfd, node_global_args.file_list[i], strlen(node_global_args.file_list[i]));

		//receive the message
		int received = recv_full_msg(socketfd, buffer, MAX_PATH_SIZE);

		//print the message
		buffer[received] = '\0'; //null character before printing the string
		fprintf(stderr, "Recieved : %s\n", buffer);

		//close the connection
		TCP_client_disconnect(socketfd);
    
	}
	
	pthread_exit(0);
	
}


int main(int argc, char* argv[]) {
    //argument check check
    if (argc != 3) {
        ERROR("Not enough arguments. Usage %s <ip_list> <file_list>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
	
    char** file_list = (char**)malloc(sizeof(char*) * (MAX_FILES));
    MALLOC_CHK(file_list);
	char* file_list_name = argv[2];
    FILE* file_list_fp = fopen(file_list_name, "r");
    NULL_CHK(file_list_fp);

    int32_t file_list_cnt = 0;
    int32_t i = 0;

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

	
	char** ip_list = (char**)malloc(sizeof(char*) * (MAX_IPS));
    MALLOC_CHK(file_list);
    //reading ip file, Contains what other hosts should be connected
    char *ip_list_name = argv[1];
	FILE *ip_list_fp = fopen(ip_list_name,"r");
	NULL_CHK(ip_list_fp);
	uint32_t ip_cnt=0;	

	while (1) {
        size_t line_size = MAX_IP_LEN;
        char* line =
            malloc(sizeof(char) * (line_size)); //filepath+newline+nullcharacter
        MALLOC_CHK(line);

        int32_t readlinebytes = getline(&line, &line_size, ip_list_fp);
        if (readlinebytes == -1) { //file has ended
            free(line);
            break;
        } else if (readlinebytes > MAX_IP_LEN) {
            free(line);
            ERROR("The length %s is longer hard coded limit %d\n", line,
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
	
    //data structures for clients (connect to servers running on other ips)
    pthread_t node_thread[MAX_IPS];     
    int32_t thread_id[MAX_IPS];
 
	node_global_args.file_list=file_list;
	node_global_args.file_list_cnt=file_list_cnt;
	node_global_args.file_list_idx=0;
	node_global_args.ip_list=ip_list;

    //First create threads for clients
    for(i=0;i<ip_cnt;i++){
        thread_id[i]=i;
        int ret = pthread_create( &node_thread[i], NULL, node_handler, (void *)(&thread_id[i])) ;
        if(ret!=0){
			perror("Error creating thread");
			exit(EXIT_FAILURE);
		}	
    }
	
	
    //joining client side threads
    for(i=0;i<ip_cnt;i++){
        int ret = pthread_join ( node_thread[i], NULL );
        if(ret!=0){
			perror("Error joining thread");
			exit(EXIT_FAILURE);
		}	
    } 
	
	//free iplist and filelist

    return 0;
}
