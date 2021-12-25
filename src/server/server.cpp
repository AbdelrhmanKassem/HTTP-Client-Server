#include "server.h"

using namespace std;

int server_fd;
atomic<int> current_clients;


/*Main function of server side
atguments are the server port
*/
int main(int argc, char **argv)
{

    int port = DEFAULT_PORT;
    if (argc == 2)
        port = atoi(argv[1]);
    printf("Starting Server on port: %d\n", port);
    init_server(port);
    printf("Server initialized on port: %d\n", port);
    int server_status;
    printf("server is listening\n");
    server_status = listen(server_fd, QUEUE_SIZE);
    if (server_status < 0)
    {
        perror("listening error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_storage connection_addr;
    socklen_t sock_size;
    int client_descriptor;
    while (true)
    {
        sock_size = sizeof(connection_addr);
        client_descriptor = accept(server_fd, (struct sockaddr *)&connection_addr, &sock_size);
        if (client_descriptor == -1)
        {
            perror("accept error");
            continue;
        }
        printf("connection started with Client %d\n", client_descriptor);

        if (current_clients == MAX_ALLOWED_CONNECTIONS)
        {
            printf("max number of clients reached a client has been denied a thread\n");
            continue;
        }
        printf("Thread Started for clinet %d\n", client_descriptor);
        /*start a thread for the client and detach it to works in parallel to the main thread*/
        thread t(connection_handler, client_descriptor);
        current_clients++;
        int x = current_clients;
        printf("number of current clients = %d\n", x);
        t.detach();
    }

    return 0;
}


/*initializes the server*/
void init_server(int port)
{
    struct sockaddr_in address;
    current_clients = 0;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    // Forcefully attaching socket to the port number
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt error");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

/*thread function which handles connection time out and request receiving */
void connection_handler(int client_descriptor)
{
    while (true)
    {
        persistent_timeout(client_descriptor);
        if (request_handler(client_descriptor) < 0)
            break;
    }
    printf("connection with client %d was terminated\n", client_descriptor);
    current_clients--;
    int x = current_clients;
    printf("number of current clients = %d\n", x);
    return;
}

/*sets the socket timeout option on receive to a value that takes into account the number of active clients*/
void persistent_timeout(int client_descriptor)
{
    struct timeval t;
    int timeout = (MAX_ALLOWED_CONNECTIONS / current_clients) * 2;
    t.tv_sec = timeout;
    t.tv_usec = 0;
    setsockopt(client_descriptor, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof t);
    printf("timeout set to: %d for client : %d\n", timeout, client_descriptor);
}


/*peeks at the recv buffer to process a request*/
int request_handler(int client_descriptor)
{
    char buffer [MAX_PAYLOAD_SIZE];
    int bytes_read = recv(client_descriptor, buffer, MAX_PAYLOAD_SIZE, MSG_PEEK);

    // timeout on recv
    if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS || errno == ETIMEDOUT)
    {
        printf("client %d stopped sending requests so his execution thread will end\n", client_descriptor);
        return -1;
    }

    // client closed the connection
    if (bytes_read == 0)
    {
        printf("client %d has termeinated the connection from his side\n", client_descriptor);
        return -1;
    }

    // incorrect request
    if (bytes_read < 4 || ((strncmp(buffer, "GET", 3) != 0) && (strncmp(buffer, "POST", 4) != 0)))
    {
        char *buf = (char *)malloc(101);
        int count = recv(client_descriptor, buf, 100, 0);
        printf("unrecognized request: %s of lenght: %d\n", buf, count);
        if (!buf)
            free(buf);
        return 0;
    }

    receive_request(buffer, client_descriptor);
    return 1;
}