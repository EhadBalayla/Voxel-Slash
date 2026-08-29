#pragma once
//literally a header file for declaring all packets LMAO

#include "Chunk.h"
#include <glm/glm.hpp>

enum class UDPPacketType : uint8_t {
    EntityTransformPacket,
};
enum class TCPPacketType : uint8_t {
    ChunkPacket,
    EntityAddPacket,
    EntityRemovePacket
};

#pragma pack(push, 1)
//UDP packets
struct EntityPacket {
    UDPPacketType type = UDPPacketType::EntityTransformPacket;
    uint64_t EntityID;
    glm::dvec3 pos;
    float rot;
};

//TCP packets
struct TCPPacketHeader {
    uint32_t packetSize;
    TCPPacketType packetType;
};
struct ChunkPacketPayload {
    int LOD;
    int64_t ChunkX, ChunkY, ChunkZ;
    BlockType m_Blocks[VOXEL_ARRAY_SIZE];
    bool HasAnything;
};
struct EntityAddPacketPayload {
    uint64_t EntityID;
};
struct EntityRemovePacketPayload {
    uint64_t EntityID;
};
#pragma pack(pop)