#ifndef SERVER_H
#define SERVER_H
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

#include "response.h"

using namespace std;

const int DEFAULT_PORT = 8080;
const int QUEUE_SIZE = 50;
const int MAX_ALLOWED_CONNECTIONS = 20;
const int MAX_PAYLOAD_SIZE = 1024*1024;

void init_server(int port);
void connection_handler(int client_descriptor);
void persistent_timeout(int client_descriptor);
int request_handler(int client_descriptor);



#endif //SERVER_H