#include "httpParser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void parse_request(char* raw_buffer, int total_len, HttpRequest* req) {
    char* line = NULL;
    char* context = NULL;

    memset(req, 0, sizeof(HttpRequest));

    if (raw_buffer == NULL || total_len == 0) return;

    char* body_ptr = strstr(raw_buffer, "\r\n\r\n");
    if (body_ptr != NULL) {
        req->body = body_ptr + 4;
        req->body_len = total_len - (int)(req->body - raw_buffer);
        *body_ptr = '\0';
    }
    else {
        req->body = NULL;
        req->body_len = 0;
    }

    char* len_str = strstr(raw_buffer, "Content-Length: ");
    if (len_str) {
        req->content_length = atoll(len_str + 16);
    }
    else {
        req->content_length = 0;
    }

    char* token = strtok_s(raw_buffer, "\r\n", &context);

    if (token == NULL) {
        return;
    }

    sscanf_s(token, "%s %s %s",
        req->method, (unsigned)sizeof(req->method),
        req->url, (unsigned)sizeof(req->url),
        req->protocol, (unsigned)sizeof(req->protocol));

    while ((token = strtok_s(NULL, "\r\n", &context)) != NULL) {
        if (strstr(token, "Range:") != NULL) {
            char* ptr = strchr(token, ':');
            if (ptr) {
                ptr++;
                while (*ptr == ' ') ptr++;
                strncpy_s(req->range, sizeof(req->range), ptr, _TRUNCATE);
            }
        }

        else if (strstr(token, "Authorization:") != NULL) {
            char* ptr = strchr(token, ':');
            if (ptr) {
                ptr++;
                while (*ptr == ' ') ptr++;
                strncpy_s(req->authorization, sizeof(req->authorization), ptr, _TRUNCATE);
            }
        }
        else if (strstr(token, "X-Video-Title:") != NULL) {
            char* ptr = strchr(token, ':');
            if (ptr) {
                ptr++;
                while (*ptr == ' ') ptr++;
                strncpy_s(req->video_title, sizeof(req->video_title), ptr, _TRUNCATE);
            }
        }
    }
}