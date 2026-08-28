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
#include "Helpers/NetworkUtilities.h"




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
    RecieveThread = std::thread(&NetworkManager::RecieveLoop, this);

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

        for(auto& e : GServer->m_EntityManager.GetAllEntities()) {
            EntityPacket data;
            data.EntityID = e.ID;
            data.pos = e.Position;
            data.rot = e.Rotation;

            sendto(UDPSocket, reinterpret_cast<char*>(&data), sizeof(EntityPacket), 0, (sockaddr*)&connection.udpAddr, sizeof(sockaddr_in));
        }

        //Entity playerEntity = GServer->m_EntityManager.GetEntity(connection.EntityID);
        //PlayerPacket data = {playerEntity.Position, playerEntity.Rotation};

        //sendto(UDPSocket, reinterpret_cast<char*>(&data), sizeof(PlayerPacket), 0, (sockaddr*)&connection.udpAddr, sizeof(sockaddr_in));

        it++;
    }
}
bool SendSingleChunk(SOCKET clientSocket, ChunkPacketPayload& packet) {
    SendTCPPacket(clientSocket, TCPPacketType::ChunkPacket, &packet, sizeof(ChunkPacketPayload));
    return true;
}
bool SendEntityAdd(SOCKET clientSocket, uint64_t ID) {
    SendTCPPacket(clientSocket, TCPPacketType::EntityAddPacket, &ID, sizeof(ID));
    return true;
}
bool SendEntityRemove(SOCKET clientSocket, uint64_t ID) {
    return true;

}
void NetworkManager::SendChunksData(SOCKET s) {
    for(auto& c : GServer->m_ChunkManager.GetChunkProvider().GetAllChunks(0)) {

        ChunkPacketPayload data;
        data.LOD = c.second->LOD;
        data.ChunkX = c.second->ChunkX;
        data.ChunkY = c.second->ChunkY;
        data.ChunkZ = c.second->ChunkZ;
        memcpy(data.m_Blocks, c.second->m_Blocks, VOXEL_ARRAY_SIZE);
        data.HasAnything = c.second->HasAnything;

        SendSingleChunk(s, data);
    }
}
void NetworkManager::SendAllEntities(SOCKET s, uint64_t IDToIgnore) {
    for(auto& c : GServer->m_EntityManager.GetAllEntities()) {
        if(c.ID != IDToIgnore) SendEntityAdd(s, c.ID);
    }
}
void NetworkManager::SendAllClientsASingleChunk(Chunk* c) {
    ChunkPacketPayload data;
    data.LOD = c->LOD;
    data.ChunkX = c->ChunkX;
    data.ChunkY = c->ChunkY;
    data.ChunkZ = c->ChunkZ;
    memcpy(data.m_Blocks, c->m_Blocks, VOXEL_ARRAY_SIZE);
    data.HasAnything = c->HasAnything;

    for(auto& client : connectedClients) {
        SendSingleChunk(client.ClientSocket, data);
    }
}
void NetworkManager::SendAllClientsEntityAdd(uint64_t ID) {
    for(auto& client : connectedClients) {
        SendEntityAdd(client.ClientSocket, ID);
    }
}
void NetworkManager::SendAllClientsEntityRemove(uint64_t ID) {
    for(auto& client : connectedClients) {
        SendEntityRemove(client.ClientSocket, ID);
    }
}





void NetworkManager::connectsLoop() {
    while(threadRunning) {
        SOCKET ClientSocket = accept(TCPSocket, NULL, NULL);
        if (ClientSocket != INVALID_SOCKET) {
            //set socket to non blocking
            u_long mode = 1;
            ioctlsocket(ClientSocket, FIONBIO, &mode); 

            char handshakeRetBuffer[20]; //just to be safe
            sockaddr_in addr;
            size_t len = sizeof(addr);
            recvfrom(UDPSocket, handshakeRetBuffer, 20, 0, (sockaddr*)&addr, (int*)&len);

            ConnectionData connection;
            connection.ClientSocket = ClientSocket;
            connection.udpAddr = addr;
            connection.EntityID = GServer->m_EntityManager.SpawnEntity("Player", glm::dvec3(10.0f, 20.0f, 10.0f));

            uint64_t NewPlayerID = connection.EntityID;
            sendto(UDPSocket, reinterpret_cast<char*>(&NewPlayerID), sizeof(uint64_t), 0, (sockaddr*)&addr, len);

            SendChunksData(connection.ClientSocket);
            SendAllEntities(connection.ClientSocket, NewPlayerID);

            std::cout << "A client connected" << std::endl;

            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                connectedClients.push_back(connection);
            }
        }
    }
}
#include "Entities/PlayerEntity.h"
void NetworkManager::RecieveLoop() {
    while(threadRunning) {
        {
            for(auto& client : connectedClients) {
                TCPRecieve(client);
            }
            UDPRecieve();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}


void NetworkManager::TCPRecieve(ConnectionData& client) {
    InputSendPacket incomingInput;
    int d = recv(client.ClientSocket, reinterpret_cast<char*>(&incomingInput), sizeof(incomingInput), 0);
    if(d == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return;
        return;
    }

    auto& entities = GServer->m_EntityManager.GetAllEntities();
    PlayerData* data = reinterpret_cast<PlayerData*>(entities[GServer->m_EntityManager.GetIDToIDX()[client.EntityID]].ExtraData);
    switch(incomingInput) {
        case InputSendPacket::ForwardPress: data->IsForward = true; break;
        case InputSendPacket::ForwardRelease: data->IsForward = false; break;
        case InputSendPacket::BackwardPress: data->IsBackward = true; break;
        case InputSendPacket::BackwardRelease: data->IsBackward = false; break;
        case InputSendPacket::LeftPress: data->IsLeft = true; break;
        case InputSendPacket::LeftRelease: data->IsLeft = false; break;
        case InputSendPacket::RightPress: data->IsRight = true; break;
        case InputSendPacket::RightRelease: data->IsRight = false; break;
        case InputSendPacket::JumpPress: data->IsJump = true; break;
        case InputSendPacket::JumpRelease: data->IsJump = false; break;
    }
}
void NetworkManager::UDPRecieve() {

}