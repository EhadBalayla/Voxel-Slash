#include "ClientNetworkManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

#define DEFAULT_BUFLEN 512

int recvbuflen = DEFAULT_BUFLEN;
char recvbuf[DEFAULT_BUFLEN];

void ClientNetworkManager::InitializeNetwork() {
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        exit(EXIT_FAILURE);
    }
}
void ClientNetworkManager::ShutdownNetwork() {
    WSACleanup();
}

void ClientNetworkManager::Connect() {
    addrinfo *result = nullptr;
    addrinfo *ptr = nullptr;
    addrinfo hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "failed to connect to server" << std::endl;
        return;
    }


    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            return;
        }

        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        return;
    }

    ThreadsRunning = true;
    recieveThread = std::thread(&ClientNetworkManager::recieveLoop, this);
    sendThread = std::thread(&ClientNetworkManager::sendLoop, this);

    connected = true;
}
void ClientNetworkManager::Disconnect() {
    ThreadsRunning = false;

    int iResult = shutdown(ConnectSocket, SD_SEND);

    sendThread.join();
    recieveThread.join();
    
    closesocket(ConnectSocket);

    connected = false;
}


void ClientNetworkManager::sendLoop() {
    while(ThreadsRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
void ClientNetworkManager::recieveLoop() {
    while(ThreadsRunning) {
        int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if(iResult > 0) {

        } 
        else if(iResult == 0) {

        }
        else {
            std::cout << WSAGetLastError() << std::endl;
        }
    }
}