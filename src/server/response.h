#ifndef RESPONSE_H
#define RESPONSE_H
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
#include <fstream>
#include <bits/stdc++.h>

#include "server.h"

using namespace std;



void receive_request(char request [], int client_descriptor);
void receive_GET_request(int client);
void receive_POST_request(int client_socketfd);
string get_file_name(string req);
void save_file(string path, string body);
string path_to_name(const std::string &path);
#endif //RESPONSE_H