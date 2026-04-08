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
    void sendLoop();
    void recieveLoop();
    std::thread recieveThread;
    std::thread sendThread;

    SOCKET ConnectSocket = INVALID_SOCKET;
};