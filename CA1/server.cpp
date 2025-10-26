
#include <string>
#include <vector>
#include "system.hpp"
#define PORT 8080

int main(){


    AirLineManagerSystem system(PORT);

    system.set_up_tcp();
    system.set_up_udp_customer();
    system.set_up_udp_airline();
    system.run();

    return 0;
}