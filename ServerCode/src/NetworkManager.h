#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <WinSock2.h>

enum InputSendPacket : uint8_t {
    ForwardPress,
    ForwardRelease,

    BackwardPress,
    BackwardRelease,

    LeftPress,
    LeftRelease,

    RightPress,
    RightRelease
};

struct ConnectionData {
    SOCKET ClientSocket = INVALID_SOCKET;
    sockaddr_in udpAddr;
    uint64_t EntityID = 0; //ID of the player entity
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void SendEntitiesData(); //sends entities data to all connected clients
    void SendChunksData(SOCKET s); //temporary, simply sends all chunks to a client
private:
    bool threadRunning = true;
    void connectsLoop();
    void TCPRecieveLoop();
    std::thread connectsThread; //a thread for listening to TCP connections
    std::thread TCPThread;

    SOCKET TCPSocket = INVALID_SOCKET;
    SOCKET UDPSocket = INVALID_SOCKET;
    std::vector<ConnectionData> connectedClients;
    std::mutex clientsMutex; //simply a mutex over the connected clients array

    std::mutex pMoveMTX;
};