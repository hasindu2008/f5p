/* @error.c
**
** A simple wrapper for error checking and logging
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void malloc_chk(void* ret, const char* func, const char* file, int line) {
    if (ret != NULL)
        return;
    fprintf(
        stderr,
        "[%s::ERROR]\033[1;31m Failed to allocate memory : "
        "%s.\033[0m\n[%s::DEBUG]\033[1;35m Error occured at %s:%d.\033[0m\n\n",
        func, strerror(errno), func, file, line);
    exit(EXIT_FAILURE);
}

void f_chk(void* ret, const char* func, const char* file, int line,
           const char* fopen_f) {
    if (ret != NULL)
        return;
    fprintf(
        stderr,
        "[%s::ERROR]\033[1;31m Failed to open %s : "
        "%s.\033[0m\n[%s::DEBUG]\033[1;35m Error occured at %s:%d.\033[0m\n\n",
        func, fopen_f, strerror(errno), func, file, line);
    exit(EXIT_FAILURE);
}

void null_chk(void* ret, const char* func, const char* file, int line) {
    if (ret != NULL)
        return;
    fprintf(stderr,
            "[%s::ERROR]\033[1;31m %s.\033[0m\n[%s::DEBUG]\033[1;35m Error "
            "occured at %s:%d.\033[0m\n\n",
            func, strerror(errno), func, file, line);
    exit(EXIT_FAILURE);
}

void neg_chk(int ret, const char* func, const char* file, int line) {
    if (ret >= 0)
        return;
    fprintf(stderr,
            "[%s::ERROR]\033[1;31m %s.\033[0m\n[%s::DEBUG]\033[1;35m Error "
            "occured at %s:%d.\033[0m\n\n",
            func, strerror(errno), func, file, line);
    exit(EXIT_FAILURE);
}
