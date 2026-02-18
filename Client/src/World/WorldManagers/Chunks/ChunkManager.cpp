#include "ChunkManager.h"

#include "../../../core/app.h"
#include "../../../core/Utilities.h"
#include "../../Chunk.h"

#include <algorithm>
#include <cmath>

#include <iostream>

ChunkManager::ChunkManager() : m_ChunkProvider(this) {
    chunksUpdater = std::thread(&ChunkManager::chunksUpdaterLoop, this);
    for(int i = 0; i < 6; i++) {
        meshIterator[i] = std::thread(&ChunkManager::chunksMeshIteratorLoop, this, i);
        uploadIterator[i] = std::thread(&ChunkManager::chunksUploadIteratorLoop, this, i);
        deletionIterator[i] = std::thread(&ChunkManager::chunksDeletionIteratorLoop, this, i);
    }
}
ChunkManager::~ChunkManager() {
    ChunkIteratorsRunning = false;
    updaterCV.notify_all();

    chunksUpdater.join();
    for(int i = 0; i < 6; i++) {
        meshIterator[i].join();
        uploadIterator[i].join();
        deletionIterator[i].join();
    }

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
            PushDeletionChunk(c);
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


void ChunkManager::PushDirtyChunk(Chunk* c) {
    std::lock_guard<std::mutex> lock(meshIteratorMTX[c->LOD]);
    meshIterationTransitionQueue[c->LOD].push(c);
}
void ChunkManager::PushUploadPending(Chunk* c) {
    std::lock_guard<std::mutex> lock(uploadIteratorMTX[c->LOD]);
    uploadIterationTransitionQueue[c->LOD].push(c);
}
void ChunkManager::PushReadyChunk(Chunk* c) {
    std::lock_guard<std::mutex> lock(readyMutex);
    readyTransitionQueue.push(c);
}
void ChunkManager::PushDeletionChunk(Chunk* c) {
    std::lock_guard<std::mutex> lock(deletionIteratorMTX[c->LOD]);
    deletionIterationTransitionQueue[c->LOD].push(c);
}



void ChunkManager::chunksUpdaterLoop() {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(tempMTX);
            updaterCV.wait(lock, [this] {return IsUpdatingChunks || !ChunkIteratorsRunning; });

            if(!ChunkIteratorsRunning) break;
        }
        for(int i = 0; i < GApp->MaxLODLevel; i++) {

            int CenterX = GApp->m_Camera.ChunkCoordX / GetLODSize(i);
            int CenterY = GApp->m_Camera.ChunkCoordY / GetLODSize(i);
            int CenterZ = GApp->m_Camera.ChunkCoordZ / GetLODSize(i);

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

void ChunkManager::chunksMeshIteratorLoop(int LOD) {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(meshIteratorMTX[LOD]);
            while(!meshIterationTransitionQueue[LOD].empty()) {
                meshPendingSet[LOD].insert(meshIterationTransitionQueue[LOD].front());
                meshIterationTransitionQueue[LOD].pop();
            }
        }

        if(!meshPendingSet[LOD].empty()) {
            for(auto it = meshPendingSet[LOD].begin(); it != meshPendingSet[LOD].end();) {
                Chunk* c = *it;
                if(!IsChunkInRenderDistance(c)) {
                    c->IsMeshPending = false;
                    c->IsDeletionPending = true;
                    PushDeletionChunk(c);
                    it = meshPendingSet[LOD].erase(it);
                    continue;
                }
                if(c->HasAnything && m_ChunkProvider.IsNeighborsReady(c)) {
                    m_ChunkProvider.MeshChunk(c);
                    it = meshPendingSet[LOD].erase(it);
                    continue;
                }
                it++;
            }
        } 
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void ChunkManager::chunksUploadIteratorLoop(int LOD) {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(uploadIteratorMTX[LOD]);
            while(!uploadIterationTransitionQueue[LOD].empty()) {
                uploadPendingSet[LOD].insert(uploadIterationTransitionQueue[LOD].front());
                uploadIterationTransitionQueue[LOD].pop();
            }
        }

        if(!uploadPendingSet[LOD].empty()) {
            for(auto it = uploadPendingSet[LOD].begin(); it != uploadPendingSet[LOD].end();) {
                Chunk* c = *it;

                if(!IsChunkInRenderDistance(c)) {
                    c->IsUploadPending = false;
                    c->IsDeletionPending = true;
                    PushDeletionChunk(c);
                    it = uploadPendingSet[LOD].erase(it);
                    continue;
                }
                if(c->GetMeshData().opaqueVerticies.size() > 0) {
                    m_ChunkProvider.UploadChunk(c); 
                    it = uploadPendingSet[LOD].erase(it);
                    continue;
                }
                it++;
            }
        } 
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void ChunkManager::chunksDeletionIteratorLoop(int LOD) {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(deletionIteratorMTX[LOD]);
            while(!deletionIterationTransitionQueue[LOD].empty()) {
                deletionPendingSet[LOD].insert(deletionIterationTransitionQueue[LOD].front());
                deletionIterationTransitionQueue[LOD].pop();
            }
        }

        if(!deletionPendingSet[LOD].empty()) {
            for(auto it = deletionPendingSet[LOD].begin(); it != deletionPendingSet[LOD].end();) {
                Chunk* c = *it;
                if(c->referenceCount <= 0) {
                    m_ChunkProvider.RemoveChunk(c);
                    it = deletionPendingSet[LOD].erase(it);
                    continue;
                }
                it++;
            }
        } 
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}