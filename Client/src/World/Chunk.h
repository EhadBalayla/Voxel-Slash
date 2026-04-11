#pragma once
#include "Block.h"
#include <vector>
#include <atomic>

#include <glm/glm.hpp>

#include "../core managers/Rendering/ChunkBuffer.h"

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
    std::vector<uint32_t> opaqueFaces;
};

class Chunk {
public:
    //raw chunk data
    int LOD;
    BlockType m_Blocks[VOXEL_ARRAY_SIZE] {BlockType::Air};

    int ChunkX, ChunkY, ChunkZ;

    //flags and side stuff
    bool IsGenerating = false;
    bool IsGenerated = false;
    bool IsMeshing = false;
    bool IsMeshed = false;
    bool IsUploading = false;
    bool IsRenderReady = false;
    bool MarkedForDeletion = false;

    bool HasAnything = false;
    bool HasOpaque = false;

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