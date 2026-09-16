#include <iostream>
#include "Server.h"

int main() {
    std::string gatheredIP;
    std::string gatheredPort;

    std::cout << "Please enter an IP Address: ";
    std::cin >> gatheredIP;
    std::cout << "Please enter a PORT: ";
    std::cin >> gatheredPort;

    ServerProperties props = {gatheredIP.data(), gatheredPort.data() };

    Server server(props); //literally starts the entire actual server LMAOOOO
    std::cout << "Hello server" << std::endl;
    while(true) {
        std::string cmd;
        std::cin >> cmd;

        if(cmd == "stop") {
            break;
        }
    }
    std::cout << "Shutting down server" << std::endl;
    return 0;
}