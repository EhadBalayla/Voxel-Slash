#include <thread>
#include <vector>
#include <mutex>
#include <WinSock2.h>

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
private:
    bool threadRunning = true;
    void connectsLoop();
    void recieveLoop();
    std::thread recieveThread; //a thread for handling data between clients
    std::thread connectsThread; //a thread for handling connections

    SOCKET ListenSocket = INVALID_SOCKET;
    std::vector<SOCKET> connectedClients;
    std::mutex clientsMutex; //simply a mutex over the connected clients array
};