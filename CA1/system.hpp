#ifndef SYSTEM_HPP
#define SYSTEM_HPP


#define REGISTER "REGISTER"
#define STDIN 0
#define STDOUT 1

#include <iostream>

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
struct Seat{

string status;
char column;
int row;
};

struct Flight{
string flight_id;
string origin;
string destination;
string time;
int capacity;
vector<Seat> seat_map;

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


bool  verify_role(int fd , string role);




int port;
int server_fd;
int socket_customer;
int socket_airline;
vector<User> users;
vector<Flight>flights;


};

#endif