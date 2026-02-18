#pragma once
#include "Block.h"
#include <vector>
#include <atomic>

#include <glm/glm.hpp>

#include "../core/Rendering/ChunkBuffer.h"

constexpr int Chunk_Length = 32; //for 32x32x32 chunks, cuz our chunks are cubic we only need one number not 3 numbers
constexpr int VOXEL_ARRAY_SIZE = Chunk_Length * Chunk_Length * Chunk_Length;

inline int IndexAt(int x, int y, int z) {
	return x + z * Chunk_Length + y * Chunk_Length * Chunk_Length;
}
inline int GetLODSize(int LOD) { return 1 << LOD; }

struct ChunkMesh {
    ChunkBuffer opaqueMeshBuffer;
    uint32_t opaqueCount;
};
struct ChunkMeshData {
    std::vector<uint32_t> opaqueVerticies;
    std::vector<uint32_t> opaqueIndicies;
};

class Chunk {
public:
    //raw chunk data
    int LOD;
    BlockType m_Blocks[VOXEL_ARRAY_SIZE] {BlockType::Air};

    int ChunkX, ChunkY, ChunkZ;

    //flags and side stuff
    bool IsGenerated = false;

    bool IsInJob = false;
    
    bool IsMeshPending = false;
    bool IsUploadPending = false;
    bool IsRenderReady = false;
    bool IsDeletionPending = false;

    bool HasAnything = false;
    bool HasOpaque = false;

    std::atomic<int> referenceCount = 0; //how many times is this chunk being referenced, for stuff like being a meshing neighbor or other stuff
    Chunk* neighbors[6] = {nullptr}; //temporary, only for meshing, not for anything else

    //functions
    void Render();

    void GenerateMeshData();
    void UploadMeshData();

    void DeleteMeshObjects();

    //getters
    glm::vec3 GetMin();
    glm::vec3 GetMax();
    ChunkMeshData& GetMeshData();
private:
    ChunkMeshData meshData;
    ChunkMesh mesh;
};