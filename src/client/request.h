#ifndef REQUEST_H
#define REQUEST_H

#include "client.h"

using namespace std;



void send_request(Command c, int client_descriptor);
void send_GET_request(Command c, int client_descriptor);
void send_POST_request(Command c, int client_descriptor);
void save_file(string path, string body);
void receive_GET_response(string file_path, int client_descriptor);
void wait_for_POST_response(int client_descriptor, string file_path);
string get_response_code(string req);
string path_to_name(const std::string &path);
#endif //REQUEST_H