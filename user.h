#ifndef USER_H
#define USER_H

#include "common.h"
#include "httpParser.h"
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

void handle_signup(SOCKET clnt_sock, HttpRequest* req);
void handle_signin(SOCKET clnt_sock, HttpRequest* req);

int get_user_id_from_token(const char* auth_header);

#endif