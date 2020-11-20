/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SC_SOCK_H
#define SC_SOCK_H

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#endif

#define SC_SOCK_BUF_SIZE 8192

enum sc_sock_op
{
    SC_SOCK_OP_UNREGISTERED = 0,
    SC_SOCK_OP_NONE = 1,
    SC_SOCK_OP_READ = 2,
    SC_SOCK_OP_WRITE = 4
};

enum sc_sock_family
{
    SC_SOCK_FAMILY_INET = AF_INET,
    SC_SOCK_FAMILY_INET6 = AF_INET6,
    SC_SOCK_FAMILY_UNIX = AF_UNIX
};

#if defined(_WIN32) || defined(_WIN64)
typedef SOCKET sc_sock_int;
#else
typedef int sc_sock_int;
#endif

struct sc_sock
{
    int type;
    bool blocking;
    sc_sock_int fd;
    int family;
    int op;
    char err[128];
};

struct sc_sock* sc_sock_create(int type, bool blocking, int family);
int sc_sock_destroy(struct sc_sock* sock);

void sc_sock_init(struct sc_sock* sock, int type, bool blocking, int family);
int sc_sock_term(struct sc_sock* sock);

int sc_sock_listen(struct sc_sock* sock, const char* host, const char* port);
int sc_sock_accept(struct sc_sock* sock, struct sc_sock* in);

int sc_sock_connect(struct sc_sock* sock, const char* addr, const char* port,
                    const char* source_addr, const char* source_port);
int sc_sock_finish_connect(struct sc_sock* sock);

int sc_sock_send(struct sc_sock* sock, char* buf, int len);
int sc_sock_recv(struct sc_sock* sock, char* buf, int len);

const char* sc_sock_error(struct sc_sock* sock);
void sc_sock_print(struct sc_sock* sock, char* buf, int len);


#endif
