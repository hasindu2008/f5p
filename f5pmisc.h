/* @f5pmisc.h
**
** miscellanous functions
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

#ifndef F5PMISC_H
#define F5PMISC_H

#include "error.h"

#include "socket.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXARG 256
static inline int system_async(char* buffer) {
    int i = 0;
    int pid;               //process id
    char* arglist[MAXARG]; //store the arguments
    char* pch;             //for strtok

    pch = strtok(buffer, " \n"); //Initiate breaking into tokens

    for (i = 0; i < MAXARG; i++) { //fill the argument list

        /*if there is a tokened argument add it to the argument list*/
        if (pch != NULL) {
            arglist[i] = malloc(sizeof(char) * 256);
            strcpy(arglist[i], pch);
            pch = strtok(NULL, " \n");
        }

        /*If end of scanned string make the entries in argument list null*/
        else {
            arglist[i] = NULL;
            break;
        }
    }

    /* fork a new process. If child replace the binary. Wait parent till child exists */
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        int check = execv(arglist[0], arglist);
        if (check == -1) { /* If cannot execute print an error and exit child*/
            fprintf(stderr,
                    "[%s::ERROR]\033[1;31m Execution failed : %s.\033[0m\n",
                    __func__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        return pid;
        //int status;
        //wait(&status); /*parent waits till child closes*/
    }
    return pid;
}

static inline int wait_async(int pid) {
    int status = 0;
    int ret = waitpid(pid, &status, 0);
    if (ret < 0) {
        perror("Waiting failed. Premature exit of a child?");
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;
    }
}

//a function to get ip using host name
static inline int get_ip(char* hostname, char* ip) {
    struct hostent* he;
    struct in_addr** addr_list;
    int i;
    if ((he = gethostbyname(hostname)) == NULL) {
        herror("gethostbyname");
        return 1;
    }
    addr_list = (struct in_addr**)he->h_addr_list;
    for (i = 0; addr_list[i] != NULL; i++) {
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }
    return 1;
}

// taken from minimap2/misc
static inline double realtime(void) {
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp, &tzp);
    return tp.tv_sec + tp.tv_usec * 1e-6;
}

// taken from minimap2/misc
static inline double cputime(void) {
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_utime.tv_sec + r.ru_stime.tv_sec +
           1e-6 * (r.ru_utime.tv_usec + r.ru_stime.tv_usec);
}

#endif
