#pragma once
#include <mutex>
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>


namespace std {
	template<>
	struct hash<glm::i64vec3> {
        std::size_t operator()(const glm::i64vec3& k) const noexcept {
            std::size_t h1 = std::hash<int64_t>()(k.x);
            std::size_t h2 = std::hash<int64_t>()(k.y);
            std::size_t h3 = std::hash<int64_t>()(k.z);

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

    Chunk* ProvideChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD);
    void UnprovideChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD);
    std::unordered_map<glm::i64vec3, Chunk*>& GetAllChunks(int LOD);
private:
    std::unordered_map<glm::i64vec3, Chunk*> chunks[6];
    std::mutex MTX[6];

    bool IsValidChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD);
    Chunk* LoadNewChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD);
};