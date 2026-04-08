#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <WinSock2.h>

struct ConnectionData {
    SOCKET ClientSocket = INVALID_SOCKET;
    uint64_t EntityID = 0; //ID of the player entity
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
private:
    bool threadRunning = true;
    void connectsLoop();
    void recieveLoop();
    void sendLoop();
    std::thread recieveThread; //a thread for handling recieving data from clients
    std::thread sendThread; // a thread for handling sending data to clients
    std::thread connectsThread; //a thread for handling connections

    SOCKET ListenSocket = INVALID_SOCKET;
    std::vector<ConnectionData> connectedClients;
    std::mutex clientsMutex; //simply a mutex over the connected clients array
};