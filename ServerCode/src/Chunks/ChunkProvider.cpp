#include "ChunkProvider.h"
#include "../Core Stuff/Chunk.h"
#include "ChunkManager.h"


ChunkProvider::ChunkProvider(ChunkManager* manager) : owningManager(manager) {}
ChunkProvider::~ChunkProvider() {}


Chunk* ChunkProvider::ProvideChunk(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int LOD) {
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        if(IsValidChunk(ChunkX, ChunkY, ChunkZ, LOD)) {
            return chunks[LOD][glm::i64vec3(ChunkX, ChunkY, ChunkZ)];
        }
    }

    Chunk* c = LoadNewChunk(ChunkX, ChunkY, ChunkZ, LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        chunks[LOD][glm::i64vec3(ChunkX, ChunkY, ChunkZ)] = c;
    }

    owningManager->GetChunkGenerator().GenerateChunk(c);
    owningManager->GetChunkGenerator().ReplaceBlocks(c);
    owningManager->GetChunkGenerator().CarveCaves(c);
    c->IsGenerated = true;

    return c;
}
void ChunkProvider::RemoveChunk(Chunk* c) {
    auto& map = GetAllChunks(c->LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[c->LOD]);
        map.erase(glm::i64vec3(c->ChunkX, c->ChunkY, c->ChunkZ));
    }
    delete c;
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