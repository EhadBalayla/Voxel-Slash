#pragma once
//literally a header file for declaring all packets LMAO

#include "Chunk.h"
#include <glm/glm.hpp>

enum class PacketType : uint8_t {
    EntityPacket,
    ChunkPacket,
};

struct PlayerPacket {
    glm::dvec3 pos;
    float rot;
};

struct ChunkPacket {
    int LOD;
    int64_t ChunkX, ChunkY, ChunkZ;
    BlockType m_Blocks[VOXEL_ARRAY_SIZE];
    bool HasAnything;
};