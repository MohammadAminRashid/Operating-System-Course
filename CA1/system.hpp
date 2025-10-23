#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#define STDIN 0
#define STDOUT 1
using namespace std; 

enum class Role {CUSTOMER,AIRLINE };
enum class Status {TEMPORARY, CONFIRMED };

struct User {
    int fd;
    Role role;
    string username;
    string password;
};
struct seat{

Status status;
char column;
int row;
};

struct Flight{
int flight_id;
string origin;
string destination;
string time;
vector<seat> seat_map;

};


class AirLineManagerSystem 
{

public:

  AirLineManagerSystem (int port);
  void set_up_udp_customer();
  void set_up_udp_airline();
  void set_up_tcp();
  void run();
 

private:
int port;
int server_fd;
int socket_customer;
int socket_airline;
vector<User> users;
vector<Flight>flights;


};

#endif