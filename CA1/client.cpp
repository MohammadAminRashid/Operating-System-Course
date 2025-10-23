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
{
    int udp_fd;
    struct sockaddr_in local_addr;

    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = 0;
    local_addr.sin_addr.s_addr = INADDR_ANY;

    bind(udp_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    return udp_fd;
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
                print(string(buff));
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