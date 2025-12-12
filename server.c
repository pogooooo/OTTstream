#include "server.h"
#include "httpParser.h"
#include "router.h"
#include "threadPool.h"
#include <stdio.h>
#include <stdlib.h>

ThreadPool g_thread_pool;

unsigned __stdcall handle_client(void* arg) {
    ClientContext* client = (ClientContext*)arg;
    SOCKET clnt_sock = client->socket;
    char buffer[BUF_SIZE];
    int str_len;
    HttpRequest req;

    str_len = recv(clnt_sock, buffer, BUF_SIZE - 1, 0);
    if (str_len > 0) {

        parse_request(buffer, str_len, &req);
        route_request(clnt_sock, &req);
    }

    closesocket(clnt_sock);
    free(client);
    return 0;
}

void start_server(SOCKET listen_sock) {
    SOCKET clnt_sock;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;

    thread_pool_init(&g_thread_pool, 8, 100);

    printf("[Server] Waiting for connections...\n");

    while (1) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(listen_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

        if (clnt_sock == INVALID_SOCKET) {
            printf("[Server] Accept failed.\n");
            continue;
        }

        ClientContext* ctx = (ClientContext*)malloc(sizeof(ClientContext));

        if (ctx == NULL) {
            closesocket(clnt_sock);
            continue;
        }

        ctx->socket = clnt_sock;
        ctx->address = clnt_addr;

        thread_pool_add_task(&g_thread_pool, handle_client, (void*)ctx);

    }

}