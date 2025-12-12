#include "common.h"
#include "dbManager.h"
#include "server.h"

int main() {
    WSADATA wsaData;
    SOCKET hServSock;
    struct sockaddr_in servAddr;

    if (init_db() != 0) {
        return -1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup() failed!\n");
        return -1;
    }

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET) {
        printf("socket() failed!\n");
        return -1;
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    if (bind(hServSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        printf("bind() failed!\n");
        return -1;
    }

    if (listen(hServSock, 5) == SOCKET_ERROR) {
        printf("listen() failed!\n");
        return -1;
    }

    start_server(hServSock);

    closesocket(hServSock);
    WSACleanup();
    return 0;
}