#include "system.hpp"
void print(string m)
{
    write(STDOUT, m.c_str(), m.size());
}
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return ""; 
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool AirLineManagerSystem ::verify_role(int fd, string role)
{
    for (auto &u : users)
    {
        if (fd == u.fd)
        {
            if (u.role == role)
            {
                return true;
            }
        }
    }

    return false;
}
vector<string> string_splitter(const string &command_line, char splitter)
{
    vector<string> words;
    string word;

    for (char c : command_line)
    {
        if (c == splitter)
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(trim(word));
    }

    return words;
}

void AirLineManagerSystem ::handle_command(string command_line, int fd)
{

    vector<string> command_words;

    command_words = string_splitter(command_line, ' ');
    // print(command_words[0]);
    // print(command_words[1]);
    // print(command_words[2]);
    // print(command_words[3]);
    //  print(to_string(command_words.size()));
    // cout << "command_words size = " << command_words.size() << endl;
    // for (int i = 0; i < command_words.size(); i++)
    //     cout << i << ": " << command_words[i] << endl;

    if (command_words[0] == REGISTER)
    {
        Flight new_flight;
        new_flight.flight_id = "fuck os";
        if (command_words.size() == 4)
        {
            bool is_already_exist = false;

            for (auto &u : users)
            {

                if (u.username == command_words[2])
                {

                    string msg = "ERROR Username already exists\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                    is_already_exist = true;
                }
            }

            //  print(to_string(u.fd));
            //   print(to_string(fd));
            if (!is_already_exist)
            {
                if (command_words[1] == "AIRLINE" or command_words[1] == "CUSTOMER")
                {
                    User new_user;
                    new_user.username = command_words[2];
                    new_user.password = command_words[3];
                    new_user.role = command_words[1];
                    users.push_back(new_user);
                    string msg = "REGISTERED OK\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
        }
    }
    else if (command_words[0] == "LOGIN")
    {

        if (command_words.size() == 3)
        {
            bool found_username = false;

            for (auto &u : users)
            {

                if (u.username == command_words[1])
                {
                    found_username = true;

                    if (u.password == command_words[2])
                    {

                        u.fd = fd;
                        string msg = "LOGIN OK\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                        break;
                    }
                    else
                    {
                        string msg = "ERROR InvalidPassword\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                        break;
                    }
                }
            }
            if (!found_username)
            {
                string msg = "ERROR UserNotFound\n";
                send(fd, msg.c_str(), msg.size(), 0);
            }
        }
    }

    else if (command_words[0] == "ADD_FLIGHT")
    {

        if (command_words.size() == 7)
        {
            if (verify_role(fd, "AIRLINE"))
            {

                Flight new_flight;
                new_flight.flight_id = command_words[1];
                new_flight.origin = command_words[2];
                new_flight.destination = command_words[3];
                new_flight.time = command_words[4];
                new_flight.capacity = stoi(command_words[5]) * stoi(command_words[6]);

                for (int i = 1; i <= stoi(command_words[5]); i++)
                {

                    for (int j = 1; j <= stoi(command_words[6]); j++)
                    {
                        Seat new_seat;
                        new_seat.status = "Free";
                        new_seat.column = 'A' + (i - 1);
                        new_seat.row = j;

                        new_flight.seat_map.push_back(new_seat);
                    }
                }

                flights.push_back(new_flight);
                string msg = "FLIGHT_ADDED OK\n";
                send(fd, msg.c_str(), msg.size(), 0);
            }
        }
    }
    else if (command_words[0] == "LIST_FLIGHT" and (verify_role(fd,"AIRLINE") or  verify_role(fd,"CUSTOMER")) and command_words.size()==1)
    {
        for (auto &f : flights)
        {

            int available = 0;
            for (auto &s : f.seat_map)
            {

                if (s.status == "Free")
                {
                    available += 1;
                }
            }

            string msg = "FLIGHT F" + f.flight_id + " " + f.origin + " " +
                         f.destination + " " + f.time + " SEATS_AVAILABLE=" + to_string(available) + "/" + to_string(f.capacity) + "\n";
            send(fd, msg.c_str(), msg.size(), 0);
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
                    handle_command(s, i);
                }
            }
        }
    }
}