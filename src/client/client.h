#ifndef CLIENT_H
#define CLIENT_H
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

using namespace std;


#define DEFAULT_PORT "8080";



struct Command{
    string request_type;
    string file_path;
    string host_name;
    string port_number = DEFAULT_PORT;
};

Command parse_command(string command);


#endif //CLIENT_H