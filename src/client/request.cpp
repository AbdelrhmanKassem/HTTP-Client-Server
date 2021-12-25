#include "request.h"

using namespace std;
const int MAX_PAYLOAD_SIZE = 1024 * 1024;

// calls the correct request handler
void send_request(Command c, int client_descriptor)
{

    if (c.request_type.compare("client_get") == 0)
        send_GET_request(c, client_descriptor);
    else if (c.request_type.compare("client_post") == 0)
        send_POST_request(c, client_descriptor);
    return;
}

// saves file named p whose content is body
void save_file(string p, string body)
{
    ofstream file(p);
    file << body;
    file.close();
}

string path_to_name(const std::string &path)
{
    auto fileNameStart = path.find_last_of("/\\");
    auto fileName = fileNameStart == std::string::npos ? path : path.substr(fileNameStart + 1);
    return fileName;
}

// char [] to string up to size
string arr_to_str(char buf[], int size)
{
    string res = "";
    for (int i = 0; i < size; i++)
        res.push_back(buf[i]);
    return res;
}

// forms the GET request and sends it to the server
void send_GET_request(Command c, int client_descriptor)
{
    string path = c.file_path;
    string msg = "GET " + path + " HTTP/1.1\r\nHOST: " + c.host_name + ":" + c.port_number + "\r\n\r\n";

    cout << "SENDING: \n"
         << msg << endl;

    send(client_descriptor, msg.c_str(), msg.length(), 0);
    printf("Get request sent\n");
    receive_GET_response(c.file_path, client_descriptor);
    return;
}

// forms the GET request and sends it to the server
void send_POST_request(Command c, int client_descriptor)
{
    string path = c.file_path;
    ifstream file;
    file.open(path);
    if (!file.is_open())
    {
        cout << "\n\ncan't open file: " << path << " POST request not sent\n\n\n"
             << endl;
        return;
    }
    stringstream stream;
    stream << file.rdbuf();
    file.close();
    string body = stream.str();

    string msg = "POST " + path + " HTTP/1.1\r\nHOST: " + c.host_name + ":" + c.port_number + "\r\nContent-Length: " + to_string(body.length()) + "\r\n\r\n";
    cout << "SENDING :\n"
         << msg << endl;

    msg += body;

    send(client_descriptor, msg.c_str(), msg.length(), 0);

    printf("file sent\n");
    wait_for_POST_response(client_descriptor, path);
}

// receive the file after sending GET request
void receive_GET_response(string file_path, int client_descriptor)
{
    char buffer[MAX_PAYLOAD_SIZE];
    int status = recv(client_descriptor, buffer, MAX_PAYLOAD_SIZE, MSG_PEEK);
    if (status < 0)
    {
        perror("error receiving GET response");
        return;
    }
    string response = arr_to_str(buffer, status);
    // cout<<"RESPONSE =====\n"<<response<<"\n======DONE"<<endl;           //FILE PRINT
    cout << "file path: " << file_path << endl;
    string msg = "HTTP/1.1 200 OK\r\n";
    if (strncmp(buffer, msg.c_str(), msg.length() - 2) != 0)
    {
        cout << "ERROR 404: FILE NOT FOUND (" << file_path << ") or error during transmission" << endl;
        recv(client_descriptor, buffer, MAX_PAYLOAD_SIZE, 0);
        return;
    }
    string req = arr_to_str(buffer, status);
    int start_position = 0, content_length = 0;
    string end = "\r\n\r\n";
    int position = 0;
    int index_str;
    while ((index_str = req.find(end, position)) != string::npos)
    {
        start_position = index_str + 4;
        position = index_str + 1;
    }
    string cl = "Content-Length: ";
    position = 0;
    int idx;
    while ((index_str = req.find(cl, position)) != string::npos)
    {
        idx = index_str + 16;
        position = index_str + 1;
    }
    while (req[idx] != '\r')
    {
        content_length *= 10;
        content_length += (req[idx] - '0');
        idx++;
    }

    char header[start_position];
    recv(client_descriptor, header, start_position, MSG_WAITALL);
    printf("file recived, start position %d, content lenght: %d\n", start_position, content_length);

    char file_buffer[content_length];
    recv(client_descriptor, file_buffer, content_length, MSG_WAITALL);
    string body = arr_to_str(file_buffer, content_length);
    save_file(path_to_name(file_path), body);
    printf("files saved!\n");
    return;
}

// wait for server to confirm file is received
void wait_for_POST_response(int client_descriptor, string file_path)
{
    struct timeval t;
    t.tv_sec = 5;
    t.tv_usec = 0;
    int res = setsockopt(client_descriptor, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof t);
    char buffer[1024];
    int bytes_read = recv(client_descriptor, buffer, 1024, 0);
    if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS || bytes_read == -1)
    {
        printf("File receive confirmation not received\n");
        return;
    }
    string response = arr_to_str(buffer, bytes_read);
    string msg = "HTTP/1.1 200 OK\r\n";
    if (strncmp(response.c_str(), msg.c_str(), msg.length() - 2) == 0)
        printf("correct message received\n");
    cout << "file received at server confirmed by : " << msg << endl;
}
