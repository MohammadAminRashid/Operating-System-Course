
#include <string>
#include <vector>
#include "system.hpp"

int main(){


    AirLineManagerSystem system(8080);

    system.set_up_tcp();
    system.run();

    return 0;
}