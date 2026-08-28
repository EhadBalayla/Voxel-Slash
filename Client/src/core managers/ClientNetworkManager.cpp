#include "ClientNetworkManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "27015"
#define IP "127.0.0.1"

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

void ClientNetworkManager::Connect() {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(27015);
    inet_pton(AF_INET, IP, &serverAddr.sin_addr); // Destination IP

    connectState = ConnectionState::Connecting;
    disconnectSleepCV.notify_one();
}
void ClientNetworkManager::Disconnect() {
    
}

void ClientNetworkManager::SendInputMode(InputSendPacket whichOne) {
    send(TCPClientSocket, reinterpret_cast<char*>(&whichOne), sizeof(InputSendPacket), 0);
}




void ClientNetworkManager::RecieveLoop() {
    bool TCPSent = false;
    bool TCPRecieved = false;
    bool UDPSent = false;
    bool UDPRecieved = false;


    while(ThreadsRunning) {
        std::unique_lock<std::mutex> lock(disconnectSleepMTX);
        disconnectSleepCV.wait(lock, [&]{return connectState != ConnectionState::Disconnected || !ThreadsRunning; });
        if (!ThreadsRunning) break;

        switch(connectState) {
            case ConnectionState::Connecting: {
                if(!TCPSent) {
                    size_t addrLen = sizeof(serverAddr);
                    connect(TCPClientSocket, (sockaddr*)&serverAddr, addrLen);
                    TCPSent = true;
                } 
                else if(!TCPRecieved) {
                    int error = 0;
                    int len = sizeof(error);

                    getsockopt(TCPClientSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
                    if(error == 0) {
                        TCPRecieved = true;
                    }
                }
                else if(!UDPSent) {
                    char MSG[] = "Hello Server";
                    sendto(UDPClientSocket, MSG, strlen(MSG), 0, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
                    UDPSent = true;
                }
                else if(!UDPRecieved) {
                    uint64_t ID;
                    size_t addrLen = sizeof(serverAddr);
                    int n = recvfrom(UDPClientSocket, reinterpret_cast<char*>(&ID), sizeof(uint64_t), 0, (sockaddr*)&serverAddr, (int*)&addrLen);
                    //if(n > 0) { 
                        UDPRecieved = true;
                        GApp->m_MPWorld->m_ClientEntityManager.PlayerEntityID = ID;
                        std::cout << "sexy here\n";
                    //}
                }

                if(TCPRecieved && UDPRecieved) connectState = ConnectionState::Connected;
                break;
            }
            case ConnectionState::Connected: {
                TCPRecieve();
                UDPRecieve();
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
            if(data->EntityID == GApp->m_MPWorld->m_ClientEntityManager.PlayerEntityID) {
                GApp->m_MPWorld->m_ClientEntityManager.playerPos = data->pos;
                GApp->m_MPWorld->m_ClientEntityManager.playerRot = data->rot;
            }
            break;
        }
    }
}
void ClientNetworkManager::TCPRecieve() {
    char tempBuffer[4096];
        
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
        TCPPacketHeader* header = reinterpret_cast<TCPPacketHeader*>(streamBuffer.data());
            
        if (streamBuffer.size() < header->packetSize) {
            break;
        }

        char* payloadStart = streamBuffer.data() + sizeof(TCPPacketHeader);
        uint32_t payloadSize = header->packetSize - sizeof(TCPPacketHeader);

        switch (header->packetType) {
                
            case TCPPacketType::ChunkPacket: {
                ChunkPacketPayload* data = reinterpret_cast<ChunkPacketPayload*>(payloadStart);
                std::cout << "Got a 100% complete chunk from server at: " 
                  << data->ChunkX << ", " << data->ChunkY << ", " << data->ChunkZ << "\n";

                GApp->m_MPWorld->m_ClientChunkManager.AddNewChunk(
                    glm::i64vec3(data->ChunkX, data->ChunkY, data->ChunkZ), 
                    data->m_Blocks, 
                    data->HasAnything
                );
                break;
            }
                
            case TCPPacketType::EntityAddPacket: {
                auto* data = reinterpret_cast<EntityAddPacketPayload*>(payloadStart);
                std::cout << "[Network] Spawning Entity ID: " << data->EntityID << "\n";
                GApp->m_MPWorld->m_ClientEntityManager.otherEntities.push_back(data->EntityID);
                break;
            }
                
            case TCPPacketType::EntityRemovePacket: {
                //auto* data = reinterpret_cast<EntityRemovePacketPayload*>(payloadStart);
                //std::cout << "[Network] Removing Entity ID: " << data->EntityID << "\n";
                break;
            }
                
            default: {
                std::cout << "[Network Warning] Unknown packet type received: " << (int)header->packetType << "\n";
                break;
            }
        }

        streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + header->packetSize);
    }
}