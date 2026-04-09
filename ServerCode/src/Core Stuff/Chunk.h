#pragma once
#include "Block.h"

constexpr int Chunk_Length = 32; //for 32x32x32 chunks, cuz our chunks are cubic we only need one number not 3 numbers
constexpr int VOXEL_ARRAY_SIZE = Chunk_Length * Chunk_Length * Chunk_Length;

inline int IndexAt(int x, int y, int z) {
	return x + z * Chunk_Length + y * Chunk_Length * Chunk_Length;
}
inline int GetLODSize(int LOD) { return 1 << LOD; }

class Chunk {
public:
    //raw chunk data
    int LOD;
    int64_t ChunkX, ChunkY, ChunkZ;
    BlockType m_Blocks[VOXEL_ARRAY_SIZE] {BlockType::Air};

    //flags and side stuff
    bool IsGenerated = false;
    bool HasAnything = false;
};