#include <iostream>
#include "Server.h"

int main() {
    Server server; //literally starts the entire actual server LMAOOOO
    server.m_ChunkManager.UpdateChunks();
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