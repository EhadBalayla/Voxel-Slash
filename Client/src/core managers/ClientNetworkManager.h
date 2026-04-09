#pragma once
#include <thread>
#include <WinSock2.h>

class ClientNetworkManager {
public:
    //for starting and ending WinSock
    void InitializeNetwork();
    void ShutdownNetwork();

    void Connect();
    void Disconnect();

    bool connected = false;
private:
    bool ThreadsRunning = true;
    
    void UDPRecieveLoop();
    std::thread UDPRecieveThread;

    SOCKET TCPClientSocket = INVALID_SOCKET;
    SOCKET UDPClientSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
};