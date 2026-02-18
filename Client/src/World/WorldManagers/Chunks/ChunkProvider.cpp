#include "ChunkProvider.h"
#include "../../Chunk.h"
#include "ChunkManager.h"

#include "../../../core/Utilities.h"

void ChunkGen(void* p1, void* p2) {
    Chunk* c = static_cast<Chunk*>(p1);
    ChunkManager* manager = static_cast<ChunkManager*>(p2);
    
    manager->GetChunkGenerator().GenerateChunk(c);
    c->IsGenerated = true;
    c->IsInJob = false;
    
    c->IsMeshPending = true; manager->PushDirtyChunk(c); //if the chunk has no BLOCK at all, dont push into meshing
}
void ChunkMesh(void* p1, void* p2) {
    Chunk* c = static_cast<Chunk*>(p1);
    ChunkManager* manager = static_cast<ChunkManager*>(p2);

    c->GenerateMeshData();
    c->IsInJob = false;

    c->IsUploadPending = true; manager->PushUploadPending(c);  //if the chunk has no mesh at all, dont push into uploading
}
void ChunkUpload(void* p1, void* p2) {
    Chunk* c = static_cast<Chunk*>(p1);
    ChunkManager* manager = static_cast<ChunkManager*>(p2);

    c->UploadMeshData();
    c->IsRenderReady = true;
    c->IsInJob = false;

    manager->PushReadyChunk(c);
}

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

    c->IsInJob = true;
    GenPool[LOD].QueueJob({ChunkGen, c, owningManager});

    return c;
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
void ChunkProvider::MeshChunk(Chunk* c) {
    c->IsMeshPending = false;
    c->IsInJob = true;
    MeshPool[c->LOD].QueueJob({ChunkMesh, c, owningManager});
}
void ChunkProvider::UploadChunk(Chunk* c) {
    c->IsUploadPending = false;
    c->IsInJob = true;
    UploadPool[c->LOD].QueueJob({ChunkUpload, c, owningManager});
}
bool ChunkProvider::IsNeighborsReady(Chunk* c) {
    int ChunkX = c->ChunkX;
    int ChunkY = c->ChunkY;
    int ChunkZ = c->ChunkZ;
    int LOD = c->LOD;

    Chunk* c1 = IsValidChunk(ChunkX - 1, ChunkY, ChunkZ, LOD) ? ProvideChunk(ChunkX - 1, ChunkY, ChunkZ, LOD) : nullptr;
    Chunk* c2 = IsValidChunk(ChunkX + 1, ChunkY, ChunkZ, LOD) ? ProvideChunk(ChunkX + 1, ChunkY, ChunkZ, LOD) : nullptr;
    Chunk* c3 = IsValidChunk(ChunkX, ChunkY - 1, ChunkZ, LOD) ? ProvideChunk(ChunkX, ChunkY - 1, ChunkZ, LOD) : nullptr;
    Chunk* c4 = IsValidChunk(ChunkX, ChunkY + 1, ChunkZ, LOD) ? ProvideChunk(ChunkX, ChunkY + 1, ChunkZ, LOD) : nullptr;
    Chunk* c5 = IsValidChunk(ChunkX, ChunkY, ChunkZ - 1, LOD) ? ProvideChunk(ChunkX, ChunkY, ChunkZ - 1, LOD) : nullptr;
    Chunk* c6 = IsValidChunk(ChunkX, ChunkY, ChunkZ + 1, LOD) ? ProvideChunk(ChunkX, ChunkY, ChunkZ + 1, LOD) : nullptr;

    bool b1 = c1 ? c1->IsGenerated : false;
    bool b2 = c2 ? c2->IsGenerated : false;
    bool b3 = c3 ? c3->IsGenerated : false;
    bool b4 = c4 ? c4->IsGenerated : false;
    bool b5 = c5 ? c5->IsGenerated : false;
    bool b6 = c6 ? c6->IsGenerated : false;

    if(b1 && b2 && b3 && b4 && b5 && b6) {
        c1->referenceCount++;
        c2->referenceCount++;
        c3->referenceCount++;
        c4->referenceCount++;
        c5->referenceCount++;
        c6->referenceCount++;

        c->neighbors[0] = c1;
        c->neighbors[1] = c2;
        c->neighbors[2] = c3;
        c->neighbors[3] = c4;
        c->neighbors[4] = c5;
        c->neighbors[5] = c6;

        return true;
    }

    return false;
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