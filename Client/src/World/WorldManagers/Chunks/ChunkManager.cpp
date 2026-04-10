#include "ChunkManager.h"

#include "../../../core managers/app.h"
#include "../../../core/Utilities.h"
#include "../../../core/LODParallelism.h"
#include "../../Chunk.h"

#include <algorithm>
#include <cmath>

#include <iostream>

ChunkManager::ChunkManager() : m_ChunkProvider(this) {
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
    chunksUpdater.join();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_ChunkProvider.DeleteAllChunks();
}

void ChunkManager::Render() {
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

        if(!IsChunkInRenderDistance(c)) {
            it = renderReadySet.erase(it);
            continue;
        }
        it++;

        if(!c->IsRenderReady) {
            c->IsUploading = true;
            c->UploadMeshData();
            c->IsUploading = false;
            c->IsRenderReady = true;
            continue;
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
    {
        std::lock_guard<std::mutex> lock(GenMTX);
        GenQueue.push(c);
    }
    GenCV.notify_one();
}
void ChunkManager::PushMesh(Chunk* c) {
    c->IsMeshing = true;
    {
        std::lock_guard<std::mutex> lock(MeshMTX);
        MeshQueue.push(c);
    }
    MeshCV.notify_one();
}
void ChunkManager::PushReady(Chunk* c) {
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

            for (int r = 0; r <= GApp->RenderDistance; r++) {

	    	    for (int dx = -r; dx <= r; dx++) {
	    	        for (int dy = -r; dy <= r; dy++) {
                        for(int dz = -r; dz <= r; dz++) {

	    		            if (dx != r && dy != r && dz != r && dx != -r && dy != -r && dz != -r) continue;
	    		            Chunk* c = m_ChunkProvider.ProvideChunk(CenterX + dx, CenterY + dy, CenterZ + dz, i);
                            if(!c->IsGenerating && !c->IsGenerated) PushGen(c);
                        }
	    	        }
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

        m_ChunkGenerator.GenerateChunk(c);
        m_ChunkGenerator.ReplaceBlocks(c);
        m_ChunkGenerator.CarveCaves(c);

        c->IsGenerated = true;
        c->IsGenerating = false;

        glm::ivec3 coords = glm::ivec3(c->ChunkX, c->ChunkY, c->ChunkZ);

        if(c->HasAnything && m_ChunkProvider.IsNeighborsReady(coords, c->LOD)) {
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

            if (ch->IsGenerated && !ch->IsMeshing && !ch->IsMeshed && ch->HasAnything && m_ChunkProvider.IsNeighborsReady(n, ch->LOD))
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

        c->GenerateMeshData();

        c->IsMeshing = false;
        c->IsMeshed = true;

        if(c->GetMeshData().opaqueFaces.size() == 0) continue;

        PushReady(c);
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