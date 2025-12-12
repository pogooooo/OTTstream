#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h> 
#include <sqlite3.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9000
#define BUF_SIZE 4096

typedef struct {
    SOCKET socket;
    struct sockaddr_in address;
} ClientContext;

#endif