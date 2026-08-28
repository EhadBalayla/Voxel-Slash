#include "NetworkUtilities.h"
#include <vector>
#include <iostream>

bool SendTCPPacket(SOCKET clientSocket, TCPPacketType type, const void* payloadData, uint32_t payloadSize) {
    const uint32_t totalSize = sizeof(TCPPacketHeader) + payloadSize;
        
    std::vector<char> sendBuffer(totalSize);
        
    TCPPacketHeader* header = reinterpret_cast<TCPPacketHeader*>(sendBuffer.data());
    header->packetSize = totalSize;
    header->packetType = type;
        
    if (payloadSize > 0 && payloadData != nullptr) {
        std::memcpy(sendBuffer.data() + sizeof(TCPPacketHeader), payloadData, payloadSize);
    }
        
    uint32_t totalBytesSent = 0;
    const char* bufferPtr = sendBuffer.data();
        
    while (totalBytesSent < totalSize) {
        int bytesRemaining = totalSize - totalBytesSent;
            
        int result = send(clientSocket, bufferPtr + totalBytesSent, bytesRemaining, 0);
            
        if (result > 0) {
            totalBytesSent += result;
        } 
        else if (result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                continue; 
            }
                
            std::cout << "Failed to send packet. WinSock Error: " << error << "\n";
            return false;
        }
        else {
            return false;
        }
    }
        
    return true;
}