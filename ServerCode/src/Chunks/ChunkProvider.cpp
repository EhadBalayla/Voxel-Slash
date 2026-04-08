#include "ChunkProvider.h"
#include "../Core Stuff/Chunk.h"
#include "ChunkManager.h"


ChunkProvider::ChunkProvider(ChunkManager* manager) : owningManager(manager) {}
ChunkProvider::~ChunkProvider() {}


Chunk* ChunkProvider::ProvideChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD) {
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        if(IsValidChunk(ChunkX, ChunkY, ChunkZ, LOD)) {
            return chunks[LOD][glm::ivec3(ChunkX, ChunkY, ChunkZ)];
        }
    }

    Chunk* c = LoadNewChunk(ChunkX, ChunkY, ChunkZ, LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[LOD]);
        chunks[LOD][glm::ivec3(ChunkX, ChunkY, ChunkZ)] = c;
    }

    owningManager->GetChunkGenerator().GenerateChunk(c);
    owningManager->GetChunkGenerator().ReplaceBlocks(c);
    owningManager->GetChunkGenerator().CarveCaves(c);

    return c;
}
void ChunkProvider::RemoveChunk(Chunk* c) {
    auto& map = GetAllChunks(c->LOD);
    {
        std::lock_guard<std::mutex> lock(MTX[c->LOD]);
        map.erase(glm::ivec3(c->ChunkX, c->ChunkY, c->ChunkZ));
    }
    delete c;
}
std::unordered_map<glm::ivec3, Chunk*>& ChunkProvider::GetAllChunks(int LOD) {
    return chunks[LOD];
}





bool ChunkProvider::IsValidChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD) {
    return chunks[LOD].find(glm::ivec3(ChunkX, ChunkY, ChunkZ)) != chunks[LOD].end();
}
Chunk* ChunkProvider::LoadNewChunk(int ChunkX, int ChunkY, int ChunkZ, int LOD) {
    Chunk* c = new Chunk;
    c->ChunkX = ChunkX;
    c->ChunkY = ChunkY;
    c->ChunkZ = ChunkZ;
    c->LOD = LOD;

    return c;
}