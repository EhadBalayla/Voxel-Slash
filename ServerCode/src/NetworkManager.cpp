#include "NetworkManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

int iResult, iSendResult;

#include "Server.h"
#include "Core Stuff/Packets.h"

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

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    //creating the TCP socket
    TCPSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (TCPSocket == INVALID_SOCKET) {
        std::cout << "Couldn't create the server socket because: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    iResult = bind( TCPSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Couldn't bind listen socket to server because: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(TCPSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);

    if (listen(TCPSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        std::cout << "Couldn't start listening because: " << WSAGetLastError() << std::endl;
        closesocket(TCPSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }


    //creating the UDP socket
    UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(UDPSocket == INVALID_SOCKET) {
        std::cout << "Couldn't create the server socket because: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(27015);
    iResult = bind(UDPSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(iResult )



    std::cout << "Starting threads for listening to connections and sending data between connected clients" << std::endl;

    connectsThread = std::thread(&NetworkManager::connectsLoop, this);
    TCPThread = std::thread(&NetworkManager::TCPRecieveLoop, this);

    std::cout << "Networking part of the server started successfully" << std::endl;
}
NetworkManager::~NetworkManager() {
    threadRunning = false;

    closesocket(TCPSocket);
    closesocket(UDPSocket);
    
    connectsThread.join();

    for(auto& n : connectedClients) {
        shutdown(n.ClientSocket, SD_SEND);
        closesocket(n.ClientSocket);
    }
    WSACleanup();
}

void NetworkManager::SendEntitiesData() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    if(connectedClients.empty()) return;

    for(auto it = connectedClients.begin(); it != connectedClients.end(); ) {
        ConnectionData connection = *it;

        Entity playerEntity = GServer->m_EntityManager.GetEntity(connection.EntityID);
        PlayerPacket data = {playerEntity.Position, playerEntity.Rotation};

        sendto(UDPSocket, reinterpret_cast<char*>(&data), sizeof(PlayerPacket), 0, (sockaddr*)&connection.udpAddr, sizeof(sockaddr_in));

        it++;
    }
}
void NetworkManager::SendChunksData(SOCKET s) {
    for(auto& c : GServer->m_ChunkManager.GetChunkProvider().GetAllChunks(0)) {

        ChunkPacket data;
        data.LOD = c.second->LOD;
        data.ChunkX = c.second->ChunkX;
        data.ChunkY = c.second->ChunkY;
        data.ChunkZ = c.second->ChunkZ;
        memcpy(data.m_Blocks, c.second->m_Blocks, VOXEL_ARRAY_SIZE);
        data.HasAnything = c.second->HasAnything;

        send(s, reinterpret_cast<char*>(&data), sizeof(ChunkPacket), 0);
    }
}





void NetworkManager::connectsLoop() {
    while(threadRunning) {
        SOCKET ClientSocket = accept(TCPSocket, NULL, NULL);
        if (ClientSocket != INVALID_SOCKET) {
            char handshakeRetBuffer[20]; //just to be safe
            sockaddr_in addr;
            size_t len = sizeof(addr);
            recvfrom(UDPSocket, handshakeRetBuffer, 20, 0, (sockaddr*)&addr, (int*)&len);

            char MSG[] = "Hello Client";
            sendto(UDPSocket, MSG, strlen(MSG), 0, (sockaddr*)&addr, len);

            std::cout << "A client connected" << std::endl;

            ConnectionData connection;
            connection.ClientSocket = ClientSocket;
            connection.udpAddr = addr;
            connection.EntityID = GServer->m_EntityManager.SpawnEntity("Player", glm::dvec3(10.0f, 20.0f, 10.0f));

            SendChunksData(connection.ClientSocket);

            std::lock_guard<std::mutex> lock(clientsMutex);
            connectedClients.push_back(connection);
        }
    }
}
#include "Entities/PlayerEntity.h"
void NetworkManager::TCPRecieveLoop() {
    while(threadRunning) {
        if(connectedClients.size() > 0) {
            InputSendPacket incomingInput;
            recv(connectedClients[0].ClientSocket, reinterpret_cast<char*>(&incomingInput), sizeof(incomingInput), MSG_WAITALL);

            std::cout << "Player Moved\n";

            auto& entities = GServer->m_EntityManager.GetAllEntities();
            PlayerData* data = reinterpret_cast<PlayerData*>(entities[GServer->m_EntityManager.GetIDToIDX()[connectedClients[0].EntityID]].ExtraData);
            switch(incomingInput) {
                case InputSendPacket::ForwardPress: data->IsForward = true; break;
                case InputSendPacket::ForwardRelease: data->IsForward = false; break;
                case InputSendPacket::BackwardPress: data->IsBackward = true; break;
                case InputSendPacket::BackwardRelease: data->IsBackward = false; break;
                case InputSendPacket::LeftPress: data->IsLeft = true; break;
                case InputSendPacket::LeftRelease: data->IsLeft = false; break;
                case InputSendPacket::RightPress: data->IsRight = true; break;
                case InputSendPacket::RightRelease: data->IsRight = false; break;
            }
        }
    }
}