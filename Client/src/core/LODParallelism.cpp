#include "LODParallelism.h"
#include "Utilities.h"
#include "../World/Chunk.h"
#include "../World/WorldManagers/Chunks/ChunkManager.h"

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


LODParallelism::LODParallelism(int LOD, ChunkManager* manager) {
    this->LOD = LOD;
    owningChunkManager = manager;
    meshIterator = std::thread(&LODParallelism::chunksMeshIteratorLoop, this);
    uploadIterator = std::thread(&LODParallelism::chunksUploadIteratorLoop, this);
    deletionIterator = std::thread(&LODParallelism::chunksDeletionIteratorLoop, this);
}
LODParallelism::~LODParallelism() {
    ChunkIteratorsRunning = false;
    meshIterator.join();
    uploadIterator.join();
    deletionIterator.join();
}

void LODParallelism::GenerateChunk(Chunk* c) {
    GenPool.QueueJob({ChunkGen, c, owningManager});
}
void LODParallelism::PushDirtyChunk(Chunk* c) {
    std::lock_guard<std::mutex> lock(meshIteratorMTX);
    meshIterationTransitionQueue.push(c);
}
void LODParallelism::PushUploadPending(Chunk* c) {
    std::lock_guard<std::mutex> lock(uploadIteratorMTX);
    uploadIterationTransitionQueue.push(c);
}
void LODParallelism::PushDeletionChunk(Chunk* c) {
    std::lock_guard<std::mutex> lock(deletionIteratorMTX[c->LOD]);
    deletionIterationTransitionQueue[c->LOD].push(c);
}

void LODParallelism::chunksMeshIteratorLoop() {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(meshIteratorMTX);
            while(!meshIterationTransitionQueue.empty()) {
                meshPendingSet.insert(meshIterationTransitionQueue.front());
                meshIterationTransitionQueue.pop();
            }
        }

        if(!meshPendingSet.empty()) {
            for(auto it = meshPendingSet.begin(); it != meshPendingSet.end();) {
                Chunk* c = *it;
                if(!IsChunkInRenderDistance(c)) {
                    c->IsMeshPending = false;
                    c->IsDeletionPending = true;
                    PushDeletionChunk(c);
                    it = meshPendingSet[LOD].erase(it);
                    continue;
                }
                if(c->HasAnything && m_ChunkProvider.IsNeighborsReady(c)) {
                    MeshChunk(c);
                    it = meshPendingSet.erase(it);
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
void LODParallelism::chunksUploadIteratorLoop() {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(uploadIteratorMTX);
            while(!uploadIterationTransitionQueue.empty()) {
                uploadPendingSet.insert(uploadIterationTransitionQueue.front());
                uploadIterationTransitionQueue.pop();
            }
        }

        if(!uploadPendingSet.empty()) {
            for(auto it = uploadPendingSet.begin(); it != uploadPendingSet.end();) {
                Chunk* c = *it;

                if(!IsChunkInRenderDistance(c)) {
                    c->IsUploadPending = false;
                    c->IsDeletionPending = true;
                    PushDeletionChunk(c);
                    it = uploadPendingSet.erase(it);
                    continue;
                }
                if(c->GetMeshData().opaqueVerticies.size() > 0) {
                    m_ChunkProvider.UploadChunk(c); 
                    it = uploadPendingSet.erase(it);
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
void LODParallelism::chunksDeletionIteratorLoop() {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(deletionIteratorMTX);
            while(!deletionIterationTransitionQueue.empty()) {
                deletionPendingSet.insert(deletionIterationTransitionQueue.front());
                deletionIterationTransitionQueue.pop();
            }
        }

        if(!deletionPendingSet.empty()) {
            for(auto it = deletionPendingSet.begin(); it != deletionPendingSet.end();) {
                Chunk* c = *it;
                if(c->referenceCount <= 0) {
                    m_ChunkProvider.RemoveChunk(c);
                    it = deletionPendingSet.erase(it);
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

void LODParallelism::MeshChunk(Chunk* c) {
    c->IsMeshPending = false;
    c->IsInJob = true;
    MeshPool.QueueJob({ChunkMesh, c, owningManager});
}
void LODParallelism::UploadChunk(Chunk* c) {
    c->IsUploadPending = false;
    c->IsInJob = true;
    UploadPool.QueueJob({ChunkUpload, c, owningManager});
}