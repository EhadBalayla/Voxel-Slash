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
//#include "Helpers/NetworkUtilities.h"




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
    unsigned long mode = 1;

    //creating the TCP socket
    TCPSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (TCPSocket == INVALID_SOCKET) {
        std::cout << "Couldn't create the server socket because: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    ioctlsocket(TCPSocket, FIONBIO, &mode);

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
    ioctlsocket(UDPSocket, FIONBIO, &mode);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(27015);
    iResult = bind(UDPSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(iResult )



    std::cout << "Starting threads for listening to connections and sending data between connected clients" << std::endl;

    //connectsThread = std::thread(&NetworkManager::connectsLoop, this);
    RecieveThread = std::thread(&NetworkManager::RecieveLoop, this);

    std::cout << "Networking part of the server started successfully" << std::endl;
}
NetworkManager::~NetworkManager() {
    threadRunning = false;

    closesocket(TCPSocket);
    closesocket(UDPSocket);
    
    RecieveThread.join();

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

        if(connection.connectionState == Connected)
        for(auto& e : GServer->m_EntityManager.GetAllEntities()) {
            EntityPacket data;
            data.EntityID = e.ID;
            data.pos = e.Position;
            data.rot = e.Rotation;

            sendto(UDPSocket, reinterpret_cast<char*>(&data), sizeof(EntityPacket), 0, (sockaddr*)&connection.udpAddr, sizeof(sockaddr_in));
        }

        it++;
    }
}
#include <zlib.h>
bool SendSingleChunk(SOCKET clientSocket, ChunkPacketPayload& packet) {
    //uLong compressedSize = compressBound(sizeof(packet));
    //unsigned char* compressedChunk = (unsigned char*)malloc(compressedSize);
    //compress(compressedChunk, &compressedSize, (Bytef*)&packet, sizeof(packet));
    
    GServer->m_NetworkManager.QueueTCPPacket(clientSocket, TCPPacketType::ChunkPacket, &packet, sizeof(packet));

    //free(compressedChunk);
    return true;
}
bool SendChunkToRemove(SOCKET clientSocket, ChunkRemovePacketPayload& packet) {
    GServer->m_NetworkManager.QueueTCPPacket(clientSocket, TCPPacketType::ChunkRemovePacket, &packet, sizeof(packet));
    return true;
}
bool SendEntityAdd(SOCKET clientSocket, uint64_t ID) {
    GServer->m_NetworkManager.QueueTCPPacket(clientSocket, TCPPacketType::EntityAddPacket, &ID, sizeof(ID));
    return true;
}
bool SendEntityRemove(SOCKET clientSocket, uint64_t ID) {
    GServer->m_NetworkManager.QueueTCPPacket(clientSocket, TCPPacketType::EntityRemovePacket, &ID, sizeof(ID));
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
        if(client.connectionState == Connected)
        SendSingleChunk(client.ClientSocket, data);
    }
}
void NetworkManager::SendAllClientsRemovingAChunk(Chunk* c) {
    ChunkRemovePacketPayload data;
    data.ChunkX = c->ChunkX;
    data.ChunkY = c->ChunkY;
    data.ChunkZ = c->ChunkZ;
    data.LOD = c->LOD;
    for(auto& client : connectedClients) {
        if(client.connectionState == Connected)
        SendChunkToRemove(client.ClientSocket, data);
    }
}
void NetworkManager::SendAllClientsEntityAdd(uint64_t ID) {
    for(auto& client : connectedClients) {
        if(client.connectionState == Connected)
        SendEntityAdd(client.ClientSocket, ID);
    }
}
void NetworkManager::SendAllClientsEntityRemove(uint64_t ID) {
    for(auto& client : connectedClients) {
        if(client.connectionState == Connected)
        SendEntityRemove(client.ClientSocket, ID);
    }
}

void NetworkManager::QueueTCPPacket(SOCKET socketTo, TCPPacketType type, void* payloadData, uint32_t payloadSize) {
    PendingTCPPacket packet;
    packet.socketTo = socketTo;
    packet.data.resize(sizeof(TCPPacketHeader) + payloadSize);
    TCPPacketHeader* header = reinterpret_cast<TCPPacketHeader*>(packet.data.data());
    header->packetSize = sizeof(TCPPacketHeader) + payloadSize;
    header->packetType = type;

    memcpy(packet.data.data() + sizeof(TCPPacketHeader), payloadData, payloadSize);

    std::lock_guard<std::mutex> lock(pendingTCPMutex);
    pendingTCPPackets.push_back(std::move(packet));
} 

#include "Entities/PlayerEntity.h"
void NetworkManager::RecieveLoop() {
    while(threadRunning) {
        FlushTCPPackets();
        SOCKET ClientSocket = accept(TCPSocket, NULL, NULL);
        if (ClientSocket != INVALID_SOCKET) {
            ConnectionData connection;
            connection.ConnectionID = connectionToken;
            connection.ClientSocket = ClientSocket;
            connectionToken++;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                connectedClients.push_back(connection);
            }
            send(ClientSocket, reinterpret_cast<char*>(&connection.ConnectionID), sizeof(uint64_t), 0);
        }

        {
            for(auto& client : connectedClients) {
                if(client.connectionState == ConnectionState::Connected) TCPRecieve(client);
            }
            UDPRecieve();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}


void NetworkManager::TCPRecieve(ConnectionData& client) {

}
void NetworkManager::UDPRecieve() {
    static char UDPReceiveBuffer[1400];
    sockaddr_in addr;
    size_t len = sizeof(addr);
    int iResult = recvfrom(UDPSocket, UDPReceiveBuffer, 1400, 0, (sockaddr*)&addr, (int*)&len);
    if(iResult <= 0) return;

    UDPPacketType type = (UDPPacketType)UDPReceiveBuffer[0];
    switch(type) {
        case UDPPacketType::HandshakePacket: {
            uint64_t ID;
            memcpy(&ID, (void*)(UDPReceiveBuffer + 1), sizeof(uint64_t));
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                for(auto& client : connectedClients) {
                    if(client.connectionState != Connected && client.ConnectionID == ID) {
                        client.udpAddr = addr;
                        client.connectionState = ConnectionState::Connected;
                        client.EntityID = GServer->m_EntityManager.SpawnEntity("Player", glm::dvec3(10.0f, 20.0f, 10.0f));
                        GServer->m_ChunkManager.UpdateChunks(0, 0, 0, 0, 0, 0);

                        sendto(UDPSocket, reinterpret_cast<char*>(&client.EntityID), sizeof(uint64_t), 0, (sockaddr*)&addr, len);
                        break;
                    }
                }
            }
            break;
        }
        case UDPPacketType::InputState: {
            InputStatePacket input;
            memcpy(&input, UDPReceiveBuffer, sizeof(input));
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                for(auto& client : connectedClients) {
                    if(client.connectionState == Connected && client.ConnectionID == input.ConnectionID) {
                        PlayerData* pData = (PlayerData*)GServer->m_EntityManager.GetEntity(client.EntityID).ExtraData;
                        pData->IsForward = input.ForwardInput;
                        pData->IsBackward = input.BackwardInput;
                        pData->IsLeft = input.LeftInput;
                        pData->IsRight = input.RightInput;

                        break;
                    }
                }
            }
            break;
        }
    }
}






void NetworkManager::FlushTCPPackets() {
    std::lock_guard<std::mutex> lock(pendingTCPMutex);
    while(!pendingTCPPackets.empty()) {
        PendingTCPPacket& packet = pendingTCPPackets.front();
        const char* data = packet.data.data() + packet.currentOffset;
        int result = send(packet.socketTo, packet.data.data() + packet.currentOffset, packet.data.size() - packet.currentOffset, 0);

        if(result > 0) {
            packet.currentOffset += result;
            if(packet.currentOffset == packet.data.size())
                pendingTCPPackets.pop_front();
            continue;
        }

        if(result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if(error == WSAEWOULDBLOCK) {
                break;
            }
        }
    }
}