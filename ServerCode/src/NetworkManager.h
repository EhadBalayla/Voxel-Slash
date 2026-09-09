#pragma once
#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <WinSock2.h>
#include "Core Stuff/Packets.h"

struct PendingTCPPacket {
    std::vector<char> data;
    size_t currentOffset = 0;
    SOCKET socketTo;
};

enum InputSendPacket : uint8_t {
    ForwardPress,
    ForwardRelease,

    BackwardPress,
    BackwardRelease,

    LeftPress,
    LeftRelease,

    RightPress,
    RightRelease,

    JumpPress,
    JumpRelease,
};

enum ConnectionState : uint8_t {
    Connecting, 
    Connected, 
    Disconnecting
};

struct ConnectionData {
    SOCKET ClientSocket = INVALID_SOCKET;
    sockaddr_in udpAddr;
    ConnectionState connectionState = ConnectionState::Connecting;
    uint64_t ConnectionID;

    uint64_t EntityID = 0; //ID of the player entity
};

class Chunk;

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void SendEntitiesData(); //sends entities data to all connected clients
    void SendChunksData(SOCKET s); //temporary, simply sends all chunks to a client
    void SendAllEntities(SOCKET s, uint64_t IDToIgnore); //temporary, simply to give an entity adding all the other entities
    void SendAllClientsASingleChunk(Chunk* c); //temporary
    void SendAllClientsRemovingAChunk(Chunk* c);
    void SendAllClientsEntityAdd(uint64_t ID);
    void SendAllClientsEntityRemove(uint64_t ID);

    void QueueTCPPacket(SOCKET socketTo, TCPPacketType type, void* payloadData, uint32_t payloadSize);
private:
    uint64_t connectionToken = 0;

    bool threadRunning = true;
    void RecieveLoop();
    std::thread connectsThread; //a thread for listening to TCP connections
    std::thread RecieveThread;

    SOCKET TCPSocket = INVALID_SOCKET;
    SOCKET UDPSocket = INVALID_SOCKET;
    std::vector<ConnectionData> connectedClients;
    std::mutex clientsMutex; //simply a mutex over the connected clients array

    std::mutex pMoveMTX;

    void TCPRecieve(ConnectionData& client);
    void UDPRecieve();


    std::deque<PendingTCPPacket> pendingTCPPackets;
    std::mutex pendingTCPMutex;
    void FlushTCPPackets();
};