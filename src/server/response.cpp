#include "response.h"

using namespace std;




//receives a request from the client and calls the right method base on the request
void receive_request(char request [], int client_descriptor)
{
    if (strncmp(request, "GET", 3) == 0)
    {
        printf("GET request recognised\n");
        receive_GET_request(client_descriptor);
        printf("GET request complete\n\n\n");
    }
    else if (strncmp(request, "POST", 4) == 0)
    {
        printf("POST request recognised\n");
        receive_POST_request(client_descriptor);
        printf("POST request complete\n\n\n");
    }
    else
        printf("Unrecognised request\n");
}

//saves file named p whose content is body
void save_file(string p, string body)
{
    
    ofstream file(p);
    file << body;
    file.close();
}

//char [] to string up to size
string arr_to_str(char buf[], int size)
{
    string res = "";
    for (int i = 0; i < size; i++)
        res.push_back(buf[i]);
    return res;
}

string path_to_name(const std::string &path)
{
    auto fileNameStart = path.find_last_of("/\\");
    auto fileName = fileNameStart == std::string::npos ? path : path.substr(fileNameStart + 1);
    return fileName;
}

//GET request handler
void receive_GET_request(int client_descriptor)
{
    char buffer [MAX_PAYLOAD_SIZE];
    int bytes_read = recv(client_descriptor, buffer, MAX_PAYLOAD_SIZE, 0);
    string request = arr_to_str(buffer, bytes_read);
    cout<<"REQUEST========"<<request<<"\n=========DONE from"<<client_descriptor<<endl;
    string path = get_file_name(request);

    ifstream file;
    file.open(path);
    if (!file.is_open())
    {
        printf("can't open file\n");
        string not_found_msg = "HTTP/1.1 404 Not Found\r\n";
        cout << "SENDING :\n" << not_found_msg << endl;
        send(client_descriptor, not_found_msg.c_str(), not_found_msg.length(), 0);
        return;
    }
    stringstream stream;
    stream << file.rdbuf();
    file.close();
    string body = stream.str();

    string msg = "HTTP/1.1 200 OK\r\nContent-Length: " + to_string(body.length()) + "\r\n\r\n";
    cout << "file found, SENDING: \n"
         << msg << "\n + file" << endl;
    msg += body;

    
    send(client_descriptor, msg.c_str(), msg.length(), 0);
    printf("file sending complete\n");
}

//POST request handler
void receive_POST_request(int client_descriptor)
{
    char buffer[MAX_PAYLOAD_SIZE];
    int status = recv(client_descriptor, buffer, MAX_PAYLOAD_SIZE, MSG_PEEK);
    if (status < 0)
    {
        perror("error receiving POST request");
        return;
    }
    string req = arr_to_str(buffer, status);
    string path = get_file_name(req);
    //cout<<"REQUEST =====\n"<<req<<"\n======DONE"<<endl;           //FILE PRINT
    cout << "POST request of file: " << path << endl;

    //finding the file start position and content lenght
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
        idx =  index_str + 16;
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

    printf("start position %d, content lenght: %d\n", start_position, content_length);

    
    char file_buffer[content_length];
    recv(client_descriptor, file_buffer, content_length, MSG_WAITALL);
    printf("file recived, start position %d, content lenght: %d\n", start_position, content_length);
    string body = arr_to_str(file_buffer, content_length);

    save_file(path_to_name(path), body);

    string msg = "HTTP/1.1 200 OK\r\n";
    cout << "SENDING : \n" << msg << endl;
    send(client_descriptor, msg.c_str(), msg.length(), 0);
    return;
}

string get_file_name(string req)
{
    istringstream s(req);
    string res;
    s >> res;
    s >> res;
    return res;
}