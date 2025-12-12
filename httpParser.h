#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

typedef struct {
    char method[16];
    char url[256];
    char protocol[16];
    char range[64];
    char authorization[512];
    char video_title[256];
    char* body;
    int body_len;
    long long content_length;
} HttpRequest;

void parse_request(char* raw_buffer, int total_len, HttpRequest* req);

#endif