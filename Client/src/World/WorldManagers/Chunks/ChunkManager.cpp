#include "ChunkManager.h"

#include "../../../core managers/app.h"
#include "../../../core/Utilities.h"
#include "../../Chunk.h"

#include <algorithm>
#include <cmath>

#include <iostream>

ChunkManager::ChunkManager() : m_ChunkProvider(this), m_ChunkGenerator(this) {
    chunksUpdater = std::thread(&ChunkManager::chunksUpdaterLoop, this);
    for(auto& t : GenThread) {
        t = std::thread(&ChunkManager::GenWorker, this);
    }
    for(auto& t : MeshThread) {
        t = std::thread(&ChunkManager::MeshWorker, this);
    }
}
ChunkManager::~ChunkManager() {
    ThreadRunning = false;
    updaterCV.notify_all();
    GenCV.notify_all();
    MeshCV.notify_all();
    chunksUpdater.join();
    for(auto& t : GenThread) {
        t.join();
    }
    for(auto& t : MeshThread) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_ChunkProvider.DeleteAllChunks();
}

void ChunkManager::Render() {
    {
        std::queue<Chunk*> temp;
        std::lock_guard<std::mutex> lock(deletionMTX);
        while(!deletionQueue.empty()) {
            Chunk* c = deletionQueue.front();

            if(IsChunkInRenderDistance(c)) { //chunk gets revived
                c->MarkedForDeletion = false;
                if(!c->IsGenerated && !c->IsGenerating) PushGen(c);
                else if(!c->IsMeshed && !c->IsMeshing && m_ChunkProvider.IsNeighborsReady(glm::ivec3(c->ChunkX, c->ChunkY, c->ChunkZ), c->LOD)) PushMesh(c);
                else if(!c->IsRenderReady && !c->IsUploading) PushReady(c);
                
                deletionQueue.pop();
                continue;
            }
            else if(!c->IsGenerating && !c->IsMeshing && !c->IsUploading) {
                m_ChunkProvider.RemoveChunk(c);

                deletionQueue.pop();
                continue;
            }

            temp.push(c);
            deletionQueue.pop();
        }
        std::swap(temp, deletionQueue);
    }

    {
        std::lock_guard<std::mutex> lock(readyMutex);
        while(!readyTransitionQueue.empty()) {
            renderReadySet.push_back(readyTransitionQueue.front());
            readyTransitionQueue.pop();
        }
    }


    GApp->m_OpaqueShader.Bind();
    for(auto it = renderReadySet.begin(); it != renderReadySet.end();) {
        Chunk* c = *it;

        if(c->MarkedForDeletion || !IsChunkInRenderDistance(c)) {
            it = renderReadySet.erase(it);
            c->IsUploading = false;
            c->IsRenderReady = false;
            continue;
        }
        it++;

        if(!c->IsRenderReady) {
            c->UploadMeshData();
            c->IsUploading = false;
            c->IsRenderReady = true;
        }

        if(!ChunkInFrustum(GApp->m_Frustum, c->GetMin(), c->GetMax())) continue;
        if(c->LOD > 0 && !ShouldLODRender(c)) continue;
        if(!c->HasAnything) continue;
        
        c->Render();
    }
}



void ChunkManager::UpdateChunks() {
    IsUpdatingChunks = true;
    updaterCV.notify_one();
}



ChunkProvider& ChunkManager::GetChunkProvider() {
    return m_ChunkProvider;
}
ChunkGenerator& ChunkManager::GetChunkGenerator() {
    return m_ChunkGenerator;
}



void ChunkManager::PushGen(Chunk* c) {
    c->IsGenerating = true;
    c->IsGenerated = false;
    {
        std::lock_guard<std::mutex> lock(GenMTX);
        GenQueue.push(c);
    }
    GenCV.notify_one();
}
void ChunkManager::PushMesh(Chunk* c) {
    c->IsMeshing = true;
    c->IsMeshed = false;
    {
        std::lock_guard<std::mutex> lock(MeshMTX);
        MeshQueue.push(c);
    }
    MeshCV.notify_one();
}
void ChunkManager::PushReady(Chunk* c) {
    c->IsUploading = true;
    c->IsRenderReady = false;
    std::lock_guard<std::mutex> lock(readyMutex);
    readyTransitionQueue.push(c);
}


void ChunkManager::chunksUpdaterLoop() {
    while(ThreadRunning) {
        {
            std::unique_lock<std::mutex> lock(tempMTX);
            updaterCV.wait(lock, [this] {return IsUpdatingChunks || !ThreadRunning; });

            if(!ThreadRunning) break;
        }
        for(int i = 0; i < GApp->MaxLODLevel; i++) {

            int CenterX = GApp->m_Player->ChunkCoordX / GetLODSize(i);
            int CenterY = GApp->m_Player->ChunkCoordY / GetLODSize(i);
            int CenterZ = GApp->m_Player->ChunkCoordZ / GetLODSize(i);

            std::unordered_set<glm::ivec3> IteratedCoords;

            for (int r = 0; r <= GApp->RenderDistance; r++) {
	    	    for (int dx = -r; dx <= r; dx++) {
	    	        for (int dy = -r; dy <= r; dy++) {
                        for(int dz = -r; dz <= r; dz++) {
	    		            if (dx != r && dy != r && dz != r && dx != -r && dy != -r && dz != -r) continue;

                            glm::ivec3 coords = glm::ivec3(CenterX + dx, CenterY + dy, CenterZ + dz);

	    		            Chunk* c = m_ChunkProvider.ProvideChunk(CenterX + dx, CenterY + dy, CenterZ + dz, i);
                            if(!c->MarkedForDeletion && !c->IsGenerating && !c->IsGenerated) PushGen(c);

                            IteratedCoords.insert(coords);
                        }
	    	        }
	    	    }
	        }

            auto& map = m_ChunkProvider.GetAllChunks(i);
            for(auto& pair : map) {
                if(IteratedCoords.find(pair.first) == IteratedCoords.end()) {
                    //chunk is outside Render Distance, mark for deletion
                    Chunk* c = pair.second;
                    c->MarkedForDeletion = true;
                    std::lock_guard<std::mutex> lock(deletionMTX);
                    deletionQueue.push(c);
                }
            }
        }

        IsUpdatingChunks = false;
    }
}
void ChunkManager::GenWorker() {
    while(ThreadRunning) {
        Chunk* c;

        {
            std::unique_lock<std::mutex> lock(GenMTX);

            GenCV.wait(lock, [this] { return !GenQueue.empty() || !ThreadRunning; });

            if (!ThreadRunning) return;

            c = GenQueue.front();
            GenQueue.pop();
        }

        if(!c->MarkedForDeletion) {
            m_ChunkGenerator.GenerateChunk(c);
            m_ChunkGenerator.ReplaceBlocks(c);
            m_ChunkGenerator.CarveCaves(c);
            c->IsGenerated = true;
        }
        c->IsGenerating = false;

        glm::ivec3 coords = glm::ivec3(c->ChunkX, c->ChunkY, c->ChunkZ);

        if(!c->MarkedForDeletion && c->HasAnything && m_ChunkProvider.IsNeighborsReady(coords, c->LOD)) {
            PushMesh(c);
        }

        static glm::ivec3 dirs[6] = {
            {1,0,0}, {-1,0,0},
            {0,1,0}, {0,-1,0},
            {0,0,1}, {0,0,-1}
        }; 

        for (auto d : dirs)
        {
            glm::ivec3 n = coords + d;

            Chunk* ch = m_ChunkProvider.GetChunk(n, c->LOD);
            if (!ch) continue;

            if (!ch->MarkedForDeletion && ch->IsGenerated && !ch->IsMeshing && !ch->IsMeshed && ch->HasAnything && m_ChunkProvider.IsNeighborsReady(n, ch->LOD))
                PushMesh(ch);
        }
    }
}
void ChunkManager::MeshWorker() {
    while(ThreadRunning) {
        Chunk* c;

        {
            std::unique_lock<std::mutex> lock(MeshMTX);

            MeshCV.wait(lock, [this] { return !MeshQueue.empty() || !ThreadRunning; });

            if (!ThreadRunning) return;

            c = MeshQueue.front();
            MeshQueue.pop();
        }

        if(!c->MarkedForDeletion) {
            c->GenerateMeshData();
            c->IsMeshed = true;
        }
        c->IsMeshing = false;

        if(!c->MarkedForDeletion && c->GetMeshData().opaqueFaces.size() > 0) PushReady(c);
    }
}


BlockType ChunkManager::GetBlockAt(int x, int y, int z) {
    int ChunkX = (int)std::floor((double)x / Chunk_Length);
    int ChunkY = (int)std::floor((double)y / Chunk_Length);
    int ChunkZ = (int)std::floor((double)z / Chunk_Length);

    int LocalX = x - ChunkX * Chunk_Length;
    int LocalY = y - ChunkY * Chunk_Length;
    int LocalZ = z - ChunkZ * Chunk_Length;

    return m_ChunkProvider.ProvideChunk(ChunkX, ChunkY, ChunkZ, 0)->m_Blocks[IndexAt(LocalX, LocalY, LocalZ)];
}
void ChunkManager::SetBlockNoUpdate(int x, int y, int z, int LOD, BlockType type) {
    
}