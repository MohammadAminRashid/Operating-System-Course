#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string>
#define STDIN 0
#define STDOUT 1
using namespace std;

void print(string m)
{
    write(STDOUT, m.c_str(), m.size());
}
int connect_tcp(int port)
{
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error in connecting to server\n");
    }
    return fd;
}

int connect_udp(int udp_port)
{   int sock;
    int  broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(udp_port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    return sock;

}

int main()
{
    int fd, udp_fd = -1;
    char buff[1024] = {0};
    int udp_port = 0;
    bool udp_connected = false;

    fd = connect_tcp(8080);

    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN, &readfds);
        FD_SET(fd, &readfds);
        int max_fd = fd;
        if (udp_connected && udp_fd > 0)
        {
            FD_SET(udp_fd, &readfds);
            if (udp_fd > max_fd)
                max_fd = udp_fd;
        }

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN, &readfds))
        {
            read(STDIN, buff, 1024);
            send(fd, buff, strlen(buff), 0);
            memset(buff, 0, 1024);
        }

        if (FD_ISSET(fd, &readfds))
        {
            int n = recv(fd, buff, 1024, 0);
            if (n > 0)
            {
                buff[n] = '\0';
                if (string(buff).substr(0, 3) == "UDP")
                {
                    udp_connected = true;
                    print("REGISTERED OK\n");
                    if (string(buff)[3] == '1')
                    {
                        udp_fd = connect_udp(8081);
                    }

                    else
                    {
                        udp_fd = connect_udp(8080);
                    }
                }
                else
                {
                    print(string(buff));
                   
                }
            }
            else if (n == 0)
            {
                print("Server disconnected.\n");
                break;
            }
            memset(buff, 0, 1024);
        }

        if (udp_connected && udp_fd > 0 && FD_ISSET(udp_fd, &readfds))
        {

            recv(udp_fd, buff, 1024, 0);
            print(string(buff));
            memset(buff, 0, 1024);
        }
    }

    if (udp_fd > 0)
        close(udp_fd);
    close(fd);
    return 0;
}
