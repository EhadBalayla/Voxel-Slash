#pragma once
#include <thread>
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
    RightRelease,

    JumpPress,
    JumpRelease,
};

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
};

class ClientNetworkManager {
public:
    //for starting and ending WinSock
    void InitializeNetwork();
    void ShutdownNetwork();

    void Connect();
    void Disconnect();

    void SendInputMode(InputSendPacket whichOne);

    ConnectionState connectState = ConnectionState::Disconnected;
private:
    bool ThreadsRunning = true;
    
    void RecieveLoop();
    std::thread ClientNetworkThread;
    std::condition_variable disconnectSleepCV;
    std::mutex disconnectSleepMTX;

    void UDPRecieve();
    void TCPRecieve();

    SOCKET TCPClientSocket = INVALID_SOCKET;
    SOCKET UDPClientSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
};