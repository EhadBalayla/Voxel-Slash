#include "NetworkManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

#define DEFAULT_BUFLEN 512

char recvbuf[DEFAULT_BUFLEN];
int iResult, iSendResult;
int recvbuflen = DEFAULT_BUFLEN;

NetworkManager::NetworkManager() {
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
    }



    addrinfo* result = nullptr;
    addrinfo* ptr = nullptr;
    addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }



    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Couldn't create the server socket because: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }



    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Couldn't bind listen socket to server because: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        std::cout << "Couldn't start listening because: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }


    std::cout << "Starting threads for listening to connections and sending data between connected clients" << std::endl;

    connectsThread = std::thread(&NetworkManager::connectsLoop, this);
    recieveThread = std::thread(&NetworkManager::recieveLoop, this);

    std::cout << "Networking part of the server started successfully" << std::endl;
}
NetworkManager::~NetworkManager() {
    threadRunning = false;

    closesocket(ListenSocket);
    
    connectsThread.join();
    recieveThread.join();

    for(auto& n : connectedClients) {
        shutdown(n, SD_SEND);
        closesocket(n);
    }
    WSACleanup();
}



void NetworkManager::connectsLoop() {
    while(threadRunning) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket != INVALID_SOCKET) {
            std::cout << "A client connected" << std::endl;
            std::lock_guard<std::mutex> lock(clientsMutex);
            connectedClients.push_back(ClientSocket);
        }
    }
}
void NetworkManager::recieveLoop() {
    while(threadRunning) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        if(connectedClients.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        for(auto it = connectedClients.begin(); it != connectedClients.end(); ) {
            SOCKET ClientSocket = *it;

            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if(iResult > 0) {
                it++;
            } 
            else {
                closesocket(ClientSocket);
                it = connectedClients.erase(it);
                std::cout << "A certain client has disconnected" << std::endl;
            }

        }
    }
}