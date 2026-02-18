#pragma once
#include "../../../core/ThreadPool.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>


namespace std {
	template<>
	struct hash<glm::ivec3> {
        std::size_t operator()(const glm::ivec3& k) const noexcept {
            std::size_t h1 = std::hash<int>()(k.x);
            std::size_t h2 = std::hash<int>()(k.y);
            std::size_t h3 = std::hash<int>()(k.z);

            // Combine the hashes (boost-style)
            std::size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}

class Chunk;
class ChunkManager;
class ChunkProvider {
public:
    ChunkProvider(ChunkManager* manager);
    ~ChunkProvider();

    ChunkManager* owningManager;

    Chunk* ProvideChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD);
    void RemoveChunk(Chunk* c);
    void MeshChunk(Chunk* c);
    void UploadChunk(Chunk* c);
    bool IsNeighborsReady(Chunk* c);
    void DeleteAllChunks();
    std::unordered_map<glm::ivec3, Chunk*>& GetAllChunks(int LOD);
private:
    std::unordered_map<glm::ivec3, Chunk*> chunks[6];
    std::mutex MTX[6];

    bool IsValidChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD);
    Chunk* LoadNewChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD);

    ThreadPool GenPool[6];
    ThreadPool MeshPool[6];
    ThreadPool UploadPool[6];
};