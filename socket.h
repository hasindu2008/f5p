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
#include <stdint.h>
/******************************* Blocking Send and Receive Functions***********************************/

/*  Function to send data

    arguments : socket - socket to which the data is to be sent
                buffer - address of the buffer containing the data to be sent (This can be any pointer casted to void*)
                length - size of the data to be sent (in bytes) 

    This function is a blocking call. It returns only when a 'length' number of bytes have been successfully sent.
    Error checking is handling inside. 
    Internally. This function first sends the number of bytes that is going to be sent. Then only, it sends the actual data.
    It will keep trying in a loop until all data is pushed. As the number of bytes is sent to the receiver first,
    receiver will be able wait until all data is received.
    
*/
void send_full_msg(int socket, void* buffer, int64_t length);

/*  Function to receive data 

    arguments : socket - socket to which the data is to be received
                buffer - address of the buffer to put in the receiving data
                length - the maximum size of the buffer (in bytes) 
    return value : the number of bytes received            

    This function is a blocking call. It returns only when a message sent through "send_full_msg" is fully received. 
    This is possible only because "send_full_msg" first send the number of bytes to be sent. This function sits in a loop until 
    the expected number of bytes is fully received. The it returns the number of received bytes
    Don't confuse this with the argument "length". Argument "length" is the max size of the buffer,
    which is needed inside the function to prevent any overflows. 
    
*/
int64_t recv_full_msg(int socket, void* buffer, int64_t length);

//similar to recv_full_msg but try only a number of times specified by 'times' before giving up
int64_t recv_full_msg_try(int socket, void* buffer, int64_t length, int times);

/***************************Server side*******************************************************/

/*Create a TCP socket on PORT
  Internally it performs binding and listening system calls
  Returns the listening socket
  */
int TCP_server_init(int PORT);

/*Accept a client through the listening socket "listenfd"
  Return the connection socket.  
*/
int TCP_server_accept_client(int listenfd);

/*Disconnect a connected client*/
void TCP_server_disconnect_client(int connectfd);

/*Close down the listening socket*/
void TCP_server_shutdown(int listenfd);

/********************************Client side***************************************************/

/* Connect to a TCP server at the port "PORT" at IP address "ip"
   returns the connection socket
*/
int TCP_client_connect(char* ip, int PORT);

//similar to TCP_client_connect, but give up after 'times' number of times
int TCP_client_connect_try(char* ip, int PORT, int times);

/* Disconnect*/
void TCP_client_disconnect(int socketfd);
