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
    LODParallels = new LODParallelism*[GApp->MaxLODLevel];
    for(int i = 0; i < GApp->MaxLODLevel; i++) {
        LODParallels[i] = new LODParallelism(i, this);
    }
}
ChunkManager::~ChunkManager() {
    ChunkIteratorsRunning = false;
    updaterCV.notify_all();

    chunksUpdater.join();
    for(int i = 0; i < GApp->MaxLODLevel; i++) {
        delete LODParallels[i];
    }
    delete[] LODParallels;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_ChunkProvider.DeleteAllChunks();
}

void ChunkManager::Render() {
    Chunk* c = nullptr;
    {
        std::lock_guard<std::mutex> lock(readyMutex);
        if(!readyTransitionQueue.empty()) {
            c = readyTransitionQueue.front();
            readyTransitionQueue.pop();
        }
    }

    if(c) renderReadySet.insert(c);


    GApp->m_Renderer.BindVoxelDescriptor();
    GApp->m_OpaqueShader.Bind();
    for(auto it = renderReadySet.begin(); it != renderReadySet.end();) {
        Chunk* c = *it;

        if(!IsChunkInRenderDistance(c)) {
            c->IsDeletionPending = true;
            LODParallels[c->LOD]->PushDeletionChunk(c);
            it = renderReadySet.erase(it);
            continue;
        }
        it++;

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


void ChunkManager::PushReadyChunk(Chunk* c) {
    std::lock_guard<std::mutex> lock(readyMutex);
    readyTransitionQueue.push(c);
}



void ChunkManager::chunksUpdaterLoop() {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(tempMTX);
            updaterCV.wait(lock, [this] {return IsUpdatingChunks || !ChunkIteratorsRunning; });

            if(!ChunkIteratorsRunning) break;
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
	    		            m_ChunkProvider.ProvideChunk(CenterX + dx, CenterY + dy, CenterZ + dz, i);
                        }
	    	        }
	    	    }
	        }
        }

        IsUpdatingChunks = false;
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