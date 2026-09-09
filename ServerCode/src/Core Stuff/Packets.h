#pragma once
//literally a header file for declaring all packets LMAO

#include "Chunk.h"
#include <glm/glm.hpp>

enum class UDPPacketType : uint8_t {
    EntityTransformPacket,
    InputState,
    HandshakePacket
};
enum class TCPPacketType : uint8_t {
    ChunkPacket,
    EntityAddPacket,
    EntityRemovePacket,
    ChunkRemovePacket
};

#pragma pack(push, 1)
//UDP packets
struct EntityPacket {
    UDPPacketType type = UDPPacketType::EntityTransformPacket;
    uint64_t EntityID;
    glm::dvec3 pos;
    float rot;
};
struct InputStatePacket {
    UDPPacketType type = UDPPacketType::InputState;
    uint64_t ConnectionID;
    bool ForwardInput;
    bool BackwardInput;
    bool LeftInput;
    bool RightInput;
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
struct ChunkRemovePacketPayload {
    int64_t ChunkX, ChunkY, ChunkZ;
    int LOD;
};
struct EntityAddPacketPayload {
    uint64_t EntityID;
};
struct EntityRemovePacketPayload {
    uint64_t EntityID;
};
#pragma pack(pop)