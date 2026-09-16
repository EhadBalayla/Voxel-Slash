#pragma once
#include <thread>
#include <mutex>
#include <WinSock2.h>
#include <vector>

enum class ClientConnectionState {
    Disconnected,
    Connecting,
    Connected,
};

class ClientNetworkManager {
public:
    //for starting and ending WinSock
    void InitializeNetwork();
    void ShutdownNetwork();

    void Connect(const char* IP, int PORT);
    void Disconnect();

    ClientConnectionState connectState = ClientConnectionState::Disconnected;
    uint64_t ClientIDInServer = 0;
    uint64_t ConnectionID = 0;

    bool IsWalkForward = false;
    bool IsWalkBackwards = false;
    bool IsWalkLeft = false;
    bool IsWalkRight = false;
private:
    bool ThreadsRunning = true;
    
    void RecieveLoop();
    std::thread ClientNetworkThread;
    std::condition_variable disconnectSleepCV;
    std::mutex disconnectSleepMTX;

    void UDPRecieve();
    void TCPRecieve();
    std::vector<char> streamBuffer;
    void SendInputSnapshot();

    SOCKET TCPClientSocket = INVALID_SOCKET;
    SOCKET UDPClientSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
};