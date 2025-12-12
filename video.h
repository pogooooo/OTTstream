#ifndef VIDEO_H
#define VIDEO_H

#include "common.h"
#include "httpParser.h"
#include <cjson/cJSON.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

void init_video_system();

void handle_video_upload(SOCKET clnt_sock, HttpRequest* req);
void handle_video_play(SOCKET clnt_sock, HttpRequest* req);

void handle_save_progress(SOCKET clnt_sock, HttpRequest* req);
void handle_get_progress(SOCKET clnt_sock, HttpRequest* req);

void handle_video_list(SOCKET clnt_sock, HttpRequest* req);
void handle_video_delete(SOCKET clnt_sock, HttpRequest* req);
void handle_video_edit(SOCKET clnt_sock, HttpRequest* req);

#endif