/* @error.h
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

#ifndef ERROR_H
#define ERROR_H

#define WARN "[%s::WARNING]\033[1;33m "
#define ERR "[%s::ERROR]\033[1;31m "
#define CEND "\033[0m\n"

#define WARNING(arg, ...)                                                      \
    fprintf(stderr, "[%s::WARNING]\033[1;33m " arg "\033[0m\n", __func__,      \
            __VA_ARGS__)
#define ERROR(arg, ...)                                                        \
    fprintf(stderr, "[%s::ERROR]\033[1;31m " arg "\033[0m\n", __func__,        \
            __VA_ARGS__)
#define INFO(arg, ...)                                                         \
    fprintf(stderr, "[%s::INFO]\033[1;34m " arg "\033[0m\n", __func__,         \
            __VA_ARGS__)
#define SUCCESS(arg, ...)                                                      \
    fprintf(stderr, "[%s::SUCCESS]\033[1;32m " arg "\033[0m\n", __func__,      \
            __VA_ARGS__)
#define DEBUG(arg, ...)                                                        \
    fprintf(stderr,                                                            \
            "[%s::DEBUG]\033[1;35m Error occured at %s:%d. " arg "\033[0m\n",  \
            __func__, __FILE__, __LINE__ - 2, __VA_ARGS__)

#define MALLOC_CHK(ret) malloc_chk((void*)ret, __func__, __FILE__, __LINE__ - 1)
#define F_CHK(ret, filename)                                                   \
    f_chk((void*)ret, __func__, __FILE__, __LINE__ - 1, filename);
#define NULL_CHK(ret) null_chk((void*)ret, __func__, __FILE__, __LINE__ - 1)
#define NEG_CHK(ret) neg_chk(ret, __func__, __FILE__, __LINE__ - 1)

void malloc_chk(void* ret, const char* func, const char* file, int line);
void f_chk(void* ret, const char* func, const char* file, int line,
           const char* fopen_f);

// Die on error. Print the error and exit if the return value of the previous function NULL
void null_chk(void* ret, const char* func, const char* file, int line);

// Die on error. Print the error and exit if the return value of the previous function is -1
void neg_chk(int ret, const char* func, const char* file, int line);

#endif
