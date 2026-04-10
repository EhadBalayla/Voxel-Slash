#include "ChunkProvider.h"
#include "../../Chunk.h"
#include "ChunkManager.h"

#include "../../../core/Utilities.h"
#include "../../../core/LODParallelism.h"


ChunkProvider::ChunkProvider(ChunkManager* manager) : owningManager(manager) {}
ChunkProvider::~ChunkProvider() {}


Chunk* ChunkProvider::ProvideChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD) {
    if(IsValidChunk(ChunkX, ChunkY, ChunkZ, LOD)) {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        return chunks[LOD][glm::ivec3(ChunkX, ChunkY, ChunkZ)];
    }

    Chunk* c = LoadNewChunk(ChunkX, ChunkY, ChunkZ, LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        chunks[LOD][glm::ivec3(ChunkX, ChunkY, ChunkZ)] = c;
    }
    return c;
}
Chunk* ChunkProvider::GetChunk(glm::ivec3 coords, int LOD) {
    std::lock_guard<std::mutex> lock(MTX[LOD]);
    if(chunks[LOD].find(coords) != chunks[LOD].end()) return chunks[LOD][coords];
    return nullptr;
}
void ChunkProvider::RemoveChunk(Chunk* c) {
    auto& map = GetAllChunks(c->LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[c->LOD]);
        map.erase(glm::ivec3(c->ChunkX, c->ChunkY, c->ChunkZ));
    }
    c->DeleteMeshObjects();
    delete c;
}
bool ChunkProvider::IsNeighborsReady(glm::ivec3 coords, int LOD) {
    static glm::ivec3 offsets[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    for (auto of : offsets) {
        Chunk* n = GetChunk(coords + of, LOD);
        if (!n || !n->IsGenerated) return false;
    }
    return true;
}
void ChunkProvider ::DeleteAllChunks() {
    for(int i = 0; i < 6; i++) {
        for(auto& n : chunks[i]) {
            Chunk* c = n.second;

            c->DeleteMeshObjects();
            delete c;
        }
    }
}
std::unordered_map<glm::ivec3, Chunk*>& ChunkProvider::GetAllChunks(int LOD) {
    return chunks[LOD];
}





bool ChunkProvider::IsValidChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD) {
    auto& map = GetAllChunks(LOD);
    std::lock_guard<std::mutex> lock(MTX[LOD]);
    return map.find(glm::ivec3(ChunkX, ChunkY, ChunkZ)) != map.end();
}
Chunk* ChunkProvider::LoadNewChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD) {
    Chunk* c = new Chunk;
    c->ChunkX = ChunkX;
    c->ChunkY = ChunkY;
    c->ChunkZ = ChunkZ;
    c->LOD = LOD;

    return c;
}