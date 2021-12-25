#include "request.h"
#include "client.h"



using namespace std;

int client_descriptor;
string commands_file = "./commands.txt";
char *server_ip;
int port;

/*client main function that read from the commands file*/
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("invalid arguments");
        exit(EXIT_FAILURE);
    }
    server_ip = argv[1];
    port = atoi(argv[2]);

    printf("starting connection to %s : port %d\n", server_ip, port);
    int sock;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error \n");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0)
    {
        perror("invalid address / Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed ");
        exit(EXIT_FAILURE);
    }
    client_descriptor = sock;
    printf("connected to %s : port %d I'm client %d\n", server_ip, port, client_descriptor);

    ifstream input_file(commands_file);
    string line;
    if (!input_file.is_open())
    {
        perror("Could not open the commands file");
        return EXIT_FAILURE;
    }

    while (getline(input_file, line))
    {
        if(line.length() < 6){
            break;
        }
        Command command = parse_command(line);
        if (command.request_type.compare("client_get") == 0)
        {
            send_request(command, client_descriptor);

        }
        else if (command.request_type.compare("client_post") == 0)
        {
            send_request(command, client_descriptor);
        }
    }
    printf("client requests done, client going to sleep for 5 minutes to exhibit the auto disconnect feature of the server");
    fflush(stdout);
    sleep(300);
    close(client_descriptor);
    return 0;
}



vector<string> tokenize(string s)
{
    vector<string> tokens;

    stringstream check1(s);

    string intermediate;

    while (getline(check1, intermediate, ' '))
    {
        tokens.push_back(intermediate);
    }
    return tokens;
}

Command parse_command(string command)
{
    vector<string> tokens = tokenize(command);
    Command res;
    res.request_type = tokens[0];
    res.file_path = tokens[1];
    res.host_name = tokens[2];
    if (tokens.size() == 4)
        res.port_number = tokens[3];
    return res;
}
