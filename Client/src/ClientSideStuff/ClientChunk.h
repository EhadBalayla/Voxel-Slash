#pragma once
#include "../World/Block.h"
#include <vector>

#include "../core managers/Rendering/ChunkBuffer.h"

inline int IndexAt(int x, int y, int z) {
	return x + z * 32 + y * 32 * 32;
}
inline int GetLODSize(int LOD) { return 1 << LOD; }

struct ChunkMesh {
    ChunkBuffer opaqueMeshBuffer;
    uint32_t opaqueCount;
};
struct ChunkMeshData {
    std::vector<uint32_t> opaqueFaces;
};

class ClientChunk {
public:
    //raw chunk data
    int LOD;
    int64_t ChunkX, ChunkY, ChunkZ;
    BlockType m_Blocks[32*32*32] {BlockType::Air};

    bool IsMeshPending = false;
    bool IsRenderReady = false;

    bool HasOpaque = false;

    //functions
    void Render();

    void GenerateMeshData();
    void UploadMeshData();
private:
    ChunkMeshData meshData;
    ChunkMesh mesh; 
};