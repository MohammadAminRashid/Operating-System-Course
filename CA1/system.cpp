#include "system.hpp"
static volatile sig_atomic_t timeout_flag = 0;

bool is_number(const string &s)
{
    if (s.empty() == true)
    {
        return false;
    }
    for (char c : s)
    {
        if (isdigit(c) == false)
            return false;
    }
    return true;
}


void AirLineManagerSystem ::set_next_alarm()
{
    int min_time = __INT_MAX__;
    for (auto &r : reserves)
    {
        if (r.time_stamp < min_time and r.status == "TEMPORARY")
        {

            min_time = r.time_stamp;
        }
    }
    alarm(min_time - time(NULL));
}

void print(string m)
{
    write(STDOUT, m.c_str(), m.size());
}
void AirLineManagerSystem ::disable_reserve()
{
    int min_time = __INT_MAX__;
    int reserve_id;
    int index_reserve;
    int i = 0;
    for (auto &r : reserves)
    {
        if (r.time_stamp < min_time and r.status == "TEMPORARY")
        {

            min_time = r.time_stamp;
            reserve_id = r.reservation_id;
            index_reserve = i;
        }
        i += 1;
    }
    for (auto &r : reserves)
    {
        if (reserve_id == r.reservation_id)
        {
            for (auto &f : flights)
            {
                if (r.flight_id == f.flight_id)
                {
                    for (auto &seat : r.seats)
                    {

                        for (auto &s : f.seat_map)
                        {

                            if (s.column == seat[0] and s.row == stoi(seat.substr(1)))
                            {
                                s.status = "Free";
                            }
                        }
                    }
                }
            }
            reserves.erase(reserves.begin() + index_reserve);
            number_of_reserves--;
            break;
        }
    }
}

void AirLineManagerSystem ::timeout(int sig)
{
    if (sig == SIGALRM)
    {
        timeout_flag = 1;
        return;
    }
}

string AirLineManagerSystem ::get_username(int fd)
{

    for (auto u : users)
    {

        if (fd == u.fd)
        {

            return u.username;
        }
    }
    return "";
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
string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
    {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
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
    if (command_words[0] == "REGISTER")
    {

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
            if (!is_already_exist)
            {
                if (command_words[1] == "AIRLINE" or command_words[1] == "CUSTOMER")
                {
                    User new_user;
                    new_user.username = command_words[2];
                    new_user.password = command_words[3];
                    new_user.role = command_words[1];
                    users.push_back(new_user);
                    if (command_words[1] == "AIRLINE")
                    {

                        string msg = "UDP1\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                    else
                    {
                        string msg = "UDP2\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    }

                    send_udp_message(socket_airline, "BROADCAST NEW_USER " + new_user.username + " " + new_user.role + "\n", port + 1);
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
                send_udp_message(socket_customer, "BROADCAST NEW_FLIGHT " + new_flight.flight_id + " " + new_flight.origin + " " + new_flight.destination + " " + new_flight.time + "\n", port);
            }
        }
    }
    else if (command_words[0] == "LIST_FLIGHT" and (verify_role(fd, "AIRLINE") or verify_role(fd, "CUSTOMER")) and command_words.size() == 1)
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
    else if (command_words[0] == "RESERVE" and command_words.size() >= 3 and verify_role(fd, "CUSTOMER"))
    {

        string flight_id = command_words[1];
        Reserve new_reserve;
        int is_ok = 1;

        for (auto &f : flights)
        {
            if (flight_id == f.flight_id)
            {

                for (int i = 2; i < command_words.size(); i++)
                {

                    string seat = command_words[i];

                    if (seat[0] >= 'A' and seat[0] <= 'Z' and is_number(seat.substr(1)))
                    {

                        for (auto &s : f.seat_map)
                        {
                            if (s.column == seat[0] and s.row == stoi(seat.substr(1)))
                            {

                                if (s.status == "Free")
                                {
                                    s.status = "WAIT";
                                    new_reserve.seats.push_back(seat);
                                    if (i == command_words.size() - 1)
                                    {
                                        new_reserve.flight_id = f.flight_id;
                                        new_reserve.reservation_id = last_reserve_id + 1;
                                        last_reserve_id++;
                                        number_of_reserves++;
                                        new_reserve.status = "TEMPORARY";
                                        new_reserve.username = get_username(fd);
                                        new_reserve.time_stamp = time(NULL) + 30;
                                        if (number_of_reserves == 1)
                                        {
                                            alarm(new_reserve.time_stamp - time(NULL));
                                        }

                                        reserves.push_back(new_reserve);
                                        string msg = "RESERVED TEMP " + to_string(new_reserve.reservation_id) + " EXPIRES_IN 30\n";
                                        send(fd, msg.c_str(), msg.size(), 0);

                                        for (auto &s : f.seat_map)
                                        {
                                            if (s.status == "WAIT")
                                            {
                                                s.status = "Reserve";
                                            }
                                        }
                                        return;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    string msg = "SEAT is Not Free\n";
                                    send(fd, msg.c_str(), msg.size(), 0);
                                    for (auto &s : f.seat_map)
                                    {
                                        if (s.status == "WAIT")
                                        {
                                            s.status = "Free";
                                        }
                                    }

                                    return;
                                }
                            }
                        }
                    }
                    else
                    {
                        string msg = "Invalid SEAT\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                        for (auto &s : f.seat_map)
                        {
                            if (s.status == "WAIT")
                            {
                                s.status = "Free";
                            }
                        }
                        return;
                    }
                }
                break;
            }
        }
    }
    else if (command_words[0] == "CONFIRM")
    {
        if (command_words.size() == 2)
        {
            int reserve_id = stoi(command_words[1]);

            for (auto &r : reserves)
            {

                if (r.reservation_id == reserve_id)
                {
                    string msg = "CONFIRMATION OK\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                    r.status = "CONFIRMED";
                    number_of_reserves--;
                    return;
                }
            }

            string msg = "ERROR ReservationExpired\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }
        return;
    }
    else if (command_words[0] == "CANCEL")
    {
        if (command_words.size() == 2)
        {
            int reserve_id = stoi(command_words[1]);
            int i = 0;
            for (auto &r : reserves)
            {
                if (reserve_id == r.reservation_id and r.status == "TEMPORARY")
                {
                    for (auto &f : flights)
                    {
                        if (r.flight_id == f.flight_id)
                        {
                            for (auto &seat : r.seats)
                            {

                                for (auto &s : f.seat_map)
                                {

                                    if (s.column == seat[0] and s.row == stoi(seat.substr(1)))
                                    {
                                        s.status = "Free";
                                    }
                                }
                            }
                        }
                    }
                    string msg = "CANCELLED OK\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                    reserves.erase(reserves.begin() + i);
                    number_of_reserves--;
                    return;
                }
                i += 1;
            }
        }
    }
}

AirLineManagerSystem ::AirLineManagerSystem(int port_)
{
    port = port_;
    last_reserve_id = 0;
    number_of_reserves = 0;
    signal(SIGALRM, timeout);

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
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");
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
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");
    bind(socket_airline, (struct sockaddr *)&bc_address, sizeof(bc_address));
}
void AirLineManagerSystem::send_udp_message(int sock, const string &message, int port)
{
    struct sockaddr_in bc_address{};
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");
    ssize_t sent_bytes = sendto(sock, message.c_str(), message.size(), 0,
                                (struct sockaddr *)&bc_address, sizeof(bc_address));
    if (sent_bytes < 0)
    {
        perror("sendto failed");
    }
}

void AirLineManagerSystem ::run()
{

    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    int max_sd = server_fd;

    while (true)
    {
        if (timeout_flag == 1)
        {
            timeout_flag = 0;
            disable_reserve();
            if (number_of_reserves >= 1)
            {
                set_next_alarm();
            }
        }
        working_set = master_set;
        int ret = select(max_sd + 1, &working_set, nullptr, nullptr, nullptr);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("select failed");
                break;
            }
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

                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                }
                else
                {
                    char buffer[1024];
                    int bytes_received = recv(i, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_received <= 0)
                    {

                        close(i);         
                        FD_CLR(i, &master_set); 
                        if (i == max_sd)
                        {
                            while (max_sd >= 0 && !FD_ISSET(max_sd, &master_set))
                            {
                                --max_sd;
                            }
                        }
                    }
                    else 
                    {
                        buffer[bytes_received] = '\0';
                        string s = string(buffer);
                        handle_command(s, i);
                    }
                }
            }
        }
    }
}