#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include "error.h"
#include "f5pmisc.h"
#include "socket.h"

#define MAX_PATH_SIZE 4096

#define MAX_FILES 4096
#define PORT 20022

int main(int argc, char *argv[]){
	
	//argument check check
	if(argc != 3){
		ERROR("Not enough arguments. Usage  %s <ip_address> <file_list>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
    
	char buffer[MAX_PATH_SIZE];
	char **file_list = (char **)malloc(sizeof(char*)*(MAX_FILES));
	MALLOC_CHK(file_list);
	

    FILE* file_list_fp = fopen(argv[2],"r");
	NULL_CHK(file_list_fp);
	
	int32_t file_list_cnt=0; 
	int32_t i=0;
	
	while(1){
		size_t line_size = MAX_PATH_SIZE;
		char *line = malloc(sizeof(char)*(line_size));  //filepath+newline+nullcharacter
		MALLOC_CHK(line);
		
		int32_t readlinebytes=getline(&line, &line_size, file_list_fp); 
		if(readlinebytes == -1){	//file has ended
			free(line);
			break;
		}
		else if(readlinebytes>MAX_PATH_SIZE){
			free(line);
			ERROR("The file path %s is longer hard coded limit %d\n",line,MAX_PATH_SIZE);
			exit(EXIT_FAILURE);
		}
		if(file_list_cnt>MAX_FILES){
			free(line);
			ERROR("The number of entries in %s exceeded the hard coded limit %d\n",argv[2],MAX_FILES);
			exit(EXIT_FAILURE);
		}
		if(line[readlinebytes-1]=='\n' || line[readlinebytes-1]=='\r'){
			line[readlinebytes-1]='\0';
		}
		file_list[file_list_cnt]=line;
		file_list_cnt++;
	}
	
	for(i=0;i<file_list_cnt;i++){
		fprintf(stderr,"%s\n",file_list[i]);
		
		//Connect to the server given as argument on port 
		int socketfd = TCP_client_connect(argv[1], PORT);
		
		//send the message
		send_full_msg(socketfd, file_list[i], strlen(file_list[i]));

		//receive the message
		int received = recv_full_msg(socketfd,buffer,MAX_PATH_SIZE);
		
		//print the message
		buffer[received]='\0';  //null character before printing the string
		fprintf(stderr,"Recieved : %s\n",buffer);
		
		//close the connection
		TCP_client_disconnect(socketfd);
	}
	



	
	return 0;
}


