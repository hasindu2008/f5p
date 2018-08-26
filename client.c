#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "socket.h"

#define BUFF_SIZE 4096


int main(int argc, char *argv[]){
	
	//argument check check
	if(argc != 3){
		fprintf(stderr, "Not enough args. Usage  %s <ip_address> <file>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
    
    char buffer[BUFF_SIZE]="Hi! I am the client. Serve me please!";
    
    char *filepath=argv[2];
	int length=strlen(filepath);
	if(length>BUFF_SIZE){
		fprintf(stderr,"Path is longer than the maximum allowed %d\n",BUFF_SIZE);
		exit(EXIT_FAILURE);
	}

    //Connect to the server given as argument on port 4777
    int socketfd = TCP_client_connect(argv[1], 4777);
	
    //send the message
    send_full_msg(socketfd, filepath, length);

    //receive the message
	int received = recv_full_msg(socketfd,buffer,BUFF_SIZE);
    
    //print the message
	buffer[received]='\0';  //null character before priniting the string
	fprintf(stderr,"Recieved : %s\n",buffer);
	
    //close the connection
	TCP_client_disconnect(socketfd);

	
	return 0;
}


