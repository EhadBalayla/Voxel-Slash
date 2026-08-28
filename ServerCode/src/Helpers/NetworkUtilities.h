#pragma once
#include "../Core Stuff/Packets.h"
#include <WinSock2.h>
#include <vector>

bool SendTCPPacket(SOCKET clientSocket, TCPPacketType type, const void* payloadData, uint32_t payloadSize);