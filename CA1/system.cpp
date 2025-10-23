#include "system.hpp"
void print(string m)
{
    write(STDOUT, m.c_str(), m.size());
}

vector<string> string_splitter(string command_line, char splitter)
{
    string word = "";
    vector<string> words;
    bool inside_quotes = false;

    for (int i = 0; i <= command_line.size(); ++i)
    {
        if (i < command_line.size())
        {
            if (command_line[i] == '"')
            {
                word += '"';
                inside_quotes = !inside_quotes;
            }
            else if (command_line[i] == splitter && !inside_quotes)
            {
                if (!word.empty())
                {
                    words.push_back(word);
                    word = "";
                }
            }
            else
            {
                word += command_line[i];
            }
        }
        else
        {
            if (!word.empty())
            {
                words.push_back(word);
            }
        }
    }

    return words;
}
void AirLineManagerSystem ::handle_command(string command_line , int fd)
{
    
    vector<string> command_words;
 
    command_words = string_splitter(command_line, ' ');
    // print(command_words[0]);
    // print(command_words[1]);
    // print(command_words[2]);
    // print(command_words[3]);
    //  print(to_string(command_words.size()));
        if (command_words[0] == REGISTER)
        {   
            if(command_words.size()==4){

                  for (auto& u :  users){

                           if(u.username==command_words[1]){

                            string msg = "ERROR Username already exists\n";
                            send(fd, msg.c_str(), msg.size(), 0);
                           }

                  }
                    for (auto& u :  users){
                            //  print(to_string(u.fd));
                            //   print(to_string(fd));
                           if(u.fd==fd){
                         
                          
                               u.username=command_words[1];
                               u.password=command_words[2];
                               u.role=command_words[3];
                               string msg = "REGISTERED OK\n";
                               send(fd, msg.c_str(), msg.size(), 0);
                               break;
                           }

                  }
            }
        }
    
    
}

AirLineManagerSystem ::AirLineManagerSystem(int port_)
{
    port = port_;
    return;
}

void AirLineManagerSystem ::set_up_tcp()
{

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 4);
    return;
}
void AirLineManagerSystem ::set_up_udp_customer()
{

    int broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;

    socket_customer = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_customer, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(socket_customer, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");
    bind(socket_customer, (struct sockaddr *)&bc_address, sizeof(bc_address));
}
void AirLineManagerSystem ::set_up_udp_airline()
{
    int broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;

    socket_airline = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_airline, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(socket_airline, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port + 1);
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");
    bind(socket_airline, (struct sockaddr *)&bc_address, sizeof(bc_address));
}

void AirLineManagerSystem ::run()
{

    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    int max_sd = server_fd;

    while (true)
    {
        working_set = master_set;
        if (select(max_sd + 1, &working_set, nullptr, nullptr, nullptr) < 0)
        {
            perror("select failed");
            break;
        }

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                if (i == server_fd)
                {
                    sockaddr_in client_addr;
                    socklen_t addrlen = sizeof(client_addr);
                    int new_socket = accept(server_fd, (sockaddr *)&client_addr, &addrlen);
                    // addClient(new_socket, type);
                    User new_user;
                    new_user.fd=new_socket;
                    users.push_back(new_user);
                    // print("Connected" + to_string(new_socket));
                  
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                }
                else
                {
                    char buffer[1024];
                    int bytes_received = recv(i, buffer, sizeof(buffer) - 1, 0);
                    buffer[bytes_received] = '\0';
                    string s = string(buffer);
                    handle_command(s,i);
                    print("hello");
                                 string msg = "ERROR Username already exists\n";
                            send(i, msg.c_str(), msg.size(), 0);
                      
                }
            }
        }
    }
}