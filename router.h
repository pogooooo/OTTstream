#ifndef ROUTER_H
#define ROUTER_H

#include "common.h"
#include "httpParser.h"

void route_request(SOCKET clnt_sock, HttpRequest* req);

#endif