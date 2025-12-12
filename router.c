#include "router.h"
#include "user.h"
#include "video.h"

void handle_404(SOCKET sock) {
    char* header = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    send(sock, header, (int)strlen(header), 0);
}

void handle_static(SOCKET sock) {
    char* header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from Router";
    send(sock, header, (int)strlen(header), 0);
}

void route_request(SOCKET clnt_sock, HttpRequest* req) {
    printf("[Router] Method: %s, URL: %s\n", req->method, req->url);

    static int init_flag = 0;
    if (!init_flag) { init_video_system(); init_flag = 1; }

    if (strcmp(req->method, "POST") == 0) {
        if (strcmp(req->url, "/auth/signup") == 0) {
            handle_signup(clnt_sock, req);
        }
        else if (strcmp(req->url, "/auth/login") == 0) {
            handle_signin(clnt_sock, req);
        }
        else if (strcmp(req->url, "/video/upload") == 0) {
            handle_video_upload(clnt_sock, req);
        }
        else if (strcmp(req->url, "/video/progress") == 0) {
            handle_save_progress(clnt_sock, req);
        }
        else if (strcmp(req->url, "/video/edit") == 0) {
            handle_video_edit(clnt_sock, req);
        }
        else {
            handle_404(clnt_sock);
        }
    }
    else if (strcmp(req->method, "GET") == 0) {
        if (strncmp(req->url, "/video/play", 11) == 0) {
            handle_video_play(clnt_sock, req);
        }
        else if (strncmp(req->url, "/video/progress", 15) == 0) {
            handle_get_progress(clnt_sock, req);
        }
        else if (strcmp(req->url, "/video/list") == 0) {
            handle_video_list(clnt_sock, req);
        }
        else {
            handle_static(clnt_sock);
        }
    }
    else if (strcmp(req->method, "DELETE") == 0) {
        if (strncmp(req->url, "/video/delete", 13) == 0) {
            handle_video_delete(clnt_sock, req);
        }
        else {
            handle_404(clnt_sock);
        }
    }
    else {
        handle_404(clnt_sock);
    }
}