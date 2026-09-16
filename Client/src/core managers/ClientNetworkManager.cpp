#include "ClientNetworkManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#undef CreateWindow
#include "app.h"
#include "Helpers/NetworkUtilities.h"




void ClientNetworkManager::InitializeNetwork() {
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        exit(EXIT_FAILURE);
    }

    //create TCP socket
    TCPClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (TCPClientSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        return;
    }
    unsigned long mode = 1;
    ioctlsocket(TCPClientSocket, FIONBIO, &mode);

    //create UDP socket
    UDPClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(UDPClientSocket == INVALID_SOCKET) {
        printf("failed to create the UDP socket\n");
        return;
    }
    ioctlsocket(UDPClientSocket, FIONBIO, &mode);

    ThreadsRunning = true;
    ClientNetworkThread = std::thread(&ClientNetworkManager::RecieveLoop, this);
}
void ClientNetworkManager::ShutdownNetwork() {
    ThreadsRunning = false;
    disconnectSleepCV.notify_one();
    ClientNetworkThread.join();

    closesocket(TCPClientSocket);
    closesocket(UDPClientSocket);
    WSACleanup();
}

void ClientNetworkManager::Connect(const char* IP, int PORT) {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &serverAddr.sin_addr); // Destination IP

    connectState = ClientConnectionState::Connecting;
    disconnectSleepCV.notify_one();
}
void ClientNetworkManager::Disconnect() {
    
}

void ClientNetworkManager::SendInputSnapshot() {
    InputStatePacket inputState;
    inputState.ConnectionID = ConnectionID;
    inputState.ForwardInput = IsWalkForward;
    inputState.BackwardInput = IsWalkBackwards;
    inputState.LeftInput = IsWalkLeft;
    inputState.RightInput = IsWalkRight;

    sendto(UDPClientSocket, (char*)(&inputState), sizeof(inputState), 0, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
}




void ClientNetworkManager::RecieveLoop() {
    bool TCPSent = false;
    bool TCPRecieved = false;
    bool UDPSent = false;
    bool UDPRecieved = false;


    while(ThreadsRunning) {
        std::unique_lock<std::mutex> lock(disconnectSleepMTX);
        disconnectSleepCV.wait(lock, [&]{return connectState != ClientConnectionState::Disconnected || !ThreadsRunning; });
        if (!ThreadsRunning) break;

        switch(connectState) {
            case ClientConnectionState::Connecting: {
                if(!TCPSent) {
                    size_t addrLen = sizeof(serverAddr);
                    connect(TCPClientSocket, (sockaddr*)&serverAddr, addrLen);
                    TCPSent = true;
                } 
                else if(!TCPRecieved) {
                    int error = 0;
                    int len = sizeof(error);

                    uint64_t buff;
                    error = recv(TCPClientSocket, reinterpret_cast<char*>(&buff), sizeof(uint64_t), 0);
                    if(error > 0) {
                        ConnectionID = buff;
                        TCPRecieved = true;
                    }
                }
                else if(!UDPSent) {
                    uint8_t MSG[9];
                    MSG[0] = (uint8_t)UDPPacketType::HandshakePacket;
                    uint64_t* MSGBuffer = reinterpret_cast<uint64_t*>(MSG + 1);
                    memcpy(MSGBuffer, &ConnectionID, sizeof(uint64_t));
                    sendto(UDPClientSocket, (char*)MSG, sizeof(MSG), 0, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
                    UDPSent = true;
                }
                else if(!UDPRecieved) {
                    uint64_t ID;
                    size_t addrLen = sizeof(serverAddr);
                    int n = recvfrom(UDPClientSocket, reinterpret_cast<char*>(&ID), sizeof(uint64_t), 0, (sockaddr*)&serverAddr, (int*)&addrLen);
                    if(n > 0) { 
                        UDPRecieved = true;
                        ClientIDInServer = ID;
                    }
                }

                if(TCPRecieved && UDPRecieved) connectState = ClientConnectionState::Connected;
                break;
            }
            case ClientConnectionState::Connected: {
                TCPRecieve();
                UDPRecieve();
                SendInputSnapshot();
                break;
            }
        }

        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}


void ClientNetworkManager::UDPRecieve() {
    static char UDPReceiveBuffer[1400];
    size_t len = sizeof(serverAddr);
    int iResult = recvfrom(UDPClientSocket, UDPReceiveBuffer, 1400, 0, (sockaddr*)&serverAddr, (int*)&len);
    
    if(iResult > 0) {
        UDPPacketType type = (UDPPacketType)UDPReceiveBuffer[0];
        switch (type) {
        case UDPPacketType::EntityTransformPacket:
            EntityPacket* data = reinterpret_cast<EntityPacket*>(UDPReceiveBuffer);
            if(data->EntityID == GApp->m_ClientNetworkManager.ClientIDInServer) {
                GApp->m_MPWorld->m_ClientEntityManager.playerPos = data->pos;
                GApp->m_MPWorld->m_ClientEntityManager.playerRot = data->rot;

                int64_t ChunkX = std::floor(data->pos.x / 32.0);
                int64_t ChunkY = std::floor(data->pos.y / 32.0);
                int64_t ChunkZ = std::floor(data->pos.z / 32.0);

                if(ChunkX != GApp->m_MPWorld->m_ClientEntityManager.CurrentChunkX ||
                ChunkY != GApp->m_MPWorld->m_ClientEntityManager.CurrentChunkY ||
                ChunkZ != GApp->m_MPWorld->m_ClientEntityManager.CurrentChunkZ) {
                    GApp->m_MPWorld->m_ClientEntityManager.CurrentChunkX = ChunkX;
                    GApp->m_MPWorld->m_ClientEntityManager.CurrentChunkY = ChunkY;
                    GApp->m_MPWorld->m_ClientEntityManager.CurrentChunkZ = ChunkZ;
                }
            }
            else {
                for(auto& e : GApp->m_MPWorld->m_ClientEntityManager.otherEntities) {
                    if(e.ID == data->EntityID) {
                        e.pos = data->pos;
                        e.rot = data->rot;
                        break;
                    }
                }
            }
            break;
        }
    }
}
#include <zlib.h>
void ClientNetworkManager::TCPRecieve() {
    char tempBuffer[8192];
        
    while (true) {
        int bytesRead = recv(TCPClientSocket, tempBuffer, sizeof(tempBuffer), 0);
            
        if (bytesRead > 0) {
            streamBuffer.insert(streamBuffer.end(), tempBuffer, tempBuffer + bytesRead);
        }
        else if (bytesRead == 0) {
            std::cout << "[Network] Connection lost (Peer disconnected).\n";
             return;
        }
        else {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                break;
            }
            std::cout << "[Network Error] recv failed with code: " << error << "\n";
            return;
        }
    }

    while (streamBuffer.size() >= sizeof(TCPPacketHeader)) {
        TCPPacketHeader header;
        memcpy(&header, streamBuffer.data(), sizeof(TCPPacketHeader));
            
        if (streamBuffer.size() < header.packetSize) {
            break;
        }
        if (header.packetSize < sizeof(TCPPacketHeader)) {
            std::cout << "[Network] Invalid TCP packet size\n";
            return;
        }

        char* payloadStart = streamBuffer.data() + sizeof(TCPPacketHeader);
        uint32_t payloadSize = header.packetSize - sizeof(TCPPacketHeader);

        switch (header.packetType) {
            case TCPPacketType::ChunkPacket: {
                ChunkPacketPayload* decompressedChunk = reinterpret_cast<ChunkPacketPayload*>(payloadStart);
                //memcpy(&decompressedChunk, payloadStart, payloadSize);
                //uLong decompressedSize = sizeof(ChunkPacketPayload);
                //int result = uncompress(reinterpret_cast<Bytef*>(&decompressedChunk), &decompressedSize, reinterpret_cast<Bytef*>(payloadStart), payloadSize); 

                GApp->m_MPWorld->m_ClientChunkManager.AddNewChunk(
                    glm::i64vec3(decompressedChunk->ChunkX, decompressedChunk->ChunkY, decompressedChunk->ChunkZ), 
                    decompressedChunk->m_Blocks, 
                    decompressedChunk->HasAnything,
                    decompressedChunk->LOD
                );
                break;
            }
                
            case TCPPacketType::EntityAddPacket: {
                auto* data = reinterpret_cast<EntityAddPacketPayload*>(payloadStart);
                GApp->m_MPWorld->m_ClientEntityManager.otherEntities.push_back({ data->EntityID, glm::dvec3(0.0), 0.0f });
                break;
            }
                
            case TCPPacketType::EntityRemovePacket: {
                auto* data = reinterpret_cast<EntityRemovePacketPayload*>(payloadStart);
                auto& entities = GApp->m_MPWorld->m_ClientEntityManager.otherEntities;
                for(auto it = entities.begin(); it != entities.end(); ) {
                    if(it->ID == data->EntityID) { 
                        entities.erase(it);
                        break;
                    }
                    ++it;
                }
                break;
            }
            case TCPPacketType::ChunkRemovePacket: {
                auto* data = reinterpret_cast<ChunkRemovePacketPayload*>(payloadStart);
                GApp->m_MPWorld->m_ClientChunkManager.RemoveChunk({data->ChunkX, data->ChunkY, data->ChunkZ}, data->LOD);
                break;
            }
                
            default: {
                std::cout << "[Network Warning] Unknown packet type received: " << (int)header.packetType << "\n";
                break;
            }
        }

        streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + header.packetSize);
    }
}