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
#include <signal.h>


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
struct Reserve
{
  int reservation_id;
  string flight_id;
  string username;
  vector<string> seats;
  string status;
  int time_stamp;
};



class AirLineManagerSystem 
{

public:

  AirLineManagerSystem (int port);
  void set_up_udp_customer(string message);
  void set_up_udp_airline(string message);
  void set_up_tcp();
  void run();
  void handle_command(string s , int fd);
  

private:


bool  verify_role(int fd , string role);
string get_username(int fd);
// void send_udp_message(int sock, const string &message, int port);
// void send_udp_message(string type, const string &message);
static void  handle_timeout(int sig);

int port;
int server_fd;
int socket_customer;
int socket_airline;
int last_reserve;
vector<User> users;
vector<Flight>flights;
vector<Reserve> reserves;


};

#endif