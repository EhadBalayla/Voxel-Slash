#include "ClientNetworkManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "27015"
#define IP "127.0.0.1"

#define DEFAULT_BUFLEN 512

int recvbuflen = DEFAULT_BUFLEN;
char recvbuf[DEFAULT_BUFLEN];

#undef CreateWindow
#include "app.h"

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

    int iResult = getaddrinfo(IP, PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "failed to connect to server" << std::endl;
        return;
    }


    //attempt TCP connection first
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        TCPClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (TCPClientSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            return;
        }

        iResult = connect( TCPClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(TCPClientSocket);
            TCPClientSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (TCPClientSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        return;
    }



    //attempt UDP connection
    UDPClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(UDPClientSocket == INVALID_SOCKET) {
        printf("failed to create the UDP socket\n");
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(27015);
    inet_pton(AF_INET, IP, &serverAddr.sin_addr); // Destination IP

    char MSG[] = "Hello Server";
    sendto(UDPClientSocket, MSG, strlen(MSG), 0, (sockaddr*)&serverAddr, sizeof(sockaddr_in));

    char handshakeRetBuffer[20]; //20 just to be safe
    size_t addrLen = sizeof(serverAddr);
    int n = recvfrom(UDPClientSocket, handshakeRetBuffer, 20, 0, (sockaddr*)&serverAddr, (int*)&addrLen);
    if(n <= 0) {
        printf("failed to get the handshake return from the server... L... \n");
        return;
    }



    ThreadsRunning = true;
    UDPRecieveThread = std::thread(&ClientNetworkManager::UDPRecieveLoop, this);

    connected = true;
}
void ClientNetworkManager::Disconnect() {
    ThreadsRunning = false;

    int iResult = shutdown(TCPClientSocket, SD_SEND);
    
    closesocket(TCPClientSocket);

    connected = false;
}

void ClientNetworkManager::UDPRecieveLoop() {
    while(ThreadsRunning) {
        struct PlayerData {
            glm::dvec3 pos;
            float rot;
        };
        PlayerData data;
        size_t len = sizeof(serverAddr);
        recvfrom(UDPClientSocket, reinterpret_cast<char*>(&data), sizeof(PlayerData), 0, (sockaddr*)&serverAddr, (int*)&len);

        GApp->m_MPWorld->m_ClientEntityManager.playerPos = data.pos;
        GApp->m_MPWorld->m_ClientEntityManager.playerRot = data.rot;
    }
}