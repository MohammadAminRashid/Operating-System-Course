#ifndef SYSTEM_HPP
#define SYSTEM_HPP


#define REGISTER "REGISTER"
#define STDIN 0
#define STDOUT 1



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

using namespace std; 



struct User {
    int fd=0;
    string role="";
    string username="";
    string password="";
};
struct seat{

string status;
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
  void handle_command(string s , int fd);
 

private:
int port;
int server_fd;
int socket_customer;
int socket_airline;
vector<User> users;
vector<Flight>flights;


};

#endif