#include "ChunkProvider.h"
#include "../Core Stuff/Chunk.h"
#include "ChunkManager.h"


ChunkProvider::ChunkProvider(ChunkManager* manager) : owningManager(manager) {}
ChunkProvider::~ChunkProvider() {}


Chunk* ChunkProvider::ProvideChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD) {
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        if(IsValidChunk(ChunkX, ChunkY, ChunkZ, LOD)) {
            Chunk* c = chunks[LOD][glm::i64vec3(ChunkX, ChunkY, ChunkZ)];
            return c;
        }
    }

    Chunk* c = LoadNewChunk(ChunkX, ChunkY, ChunkZ, LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        chunks[LOD][glm::i64vec3(ChunkX, ChunkY, ChunkZ)] = c;
    }
    return c;
}
#include <iostream>
#include "../Server.h"
void ChunkProvider::UnprovideChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD) {
    auto& map = GetAllChunks(LOD);
    Chunk* c;
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        c = map[glm::i64vec3(ChunkX, ChunkY, ChunkZ)];
        map.erase(glm::i64vec3(ChunkX, ChunkY, ChunkZ));
    }
    GServer->m_NetworkManager.SendAllClientsRemovingAChunk(c);
    delete c;
    std::cout << "[SERVER]: Deleted chunk at: " << ChunkX << ", " << ChunkY << ", " << ChunkZ << "... at LOD level: " << LOD <<"\n";
}
std::unordered_map<glm::i64vec3, Chunk*>& ChunkProvider::GetAllChunks(int LOD) {
    return chunks[LOD];
}





bool ChunkProvider::IsValidChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD) {
    return chunks[LOD].find(glm::i64vec3(ChunkX, ChunkY, ChunkZ)) != chunks[LOD].end();
}
Chunk* ChunkProvider::LoadNewChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD) {
    Chunk* c = new Chunk;
    c->ChunkX = ChunkX;
    c->ChunkY = ChunkY;
    c->ChunkZ = ChunkZ;
    c->LOD = LOD;

    return c;
}