#include "ClientChunkManager.h"
#include "ClientChunk.h"

#include "../core managers/app.h"

ClientChunkManager::ClientChunkManager() {
    meshIterator = std::thread(&ClientChunkManager::MeshIteratorLoop, this);
}
ClientChunkManager::~ClientChunkManager() {
    ThreadRunning = false;
    meshIterator.join();
}

void ClientChunkManager::AddNewChunk(glm::i64vec3 coords, void* data) {
    if(LoadedChunks.find(coords) != LoadedChunks.end()) return;

    ClientChunk* c = new ClientChunk;
    c->ChunkX = coords.x;
    c->ChunkY = coords.y;
    c->ChunkZ = coords.z;
    c->LOD = 0;

    memcpy((void*)c->m_Blocks, data, 32*32*32);

    c->IsMeshPending = true;

    LoadedChunks[coords] = c;
    PushMeshPending(c);
}

void ClientChunkManager::RenderChunks() {
    ClientChunk* c = nullptr;
    {
        std::lock_guard<std::mutex> lock(readyMutex);
        if(!readyTransitionQueue.empty()) {
            c = readyTransitionQueue.front();
            readyTransitionQueue.pop();
        }
    }

    if(c) RenderReadySet.insert(c);

    GApp->m_OpaqueShader.Bind();
    for(auto it = RenderReadySet.begin(); it != RenderReadySet.end();) {
        ClientChunk* c = *it;

        if(!c->IsRenderReady) {
            c->UploadMeshData();
            c->IsRenderReady = true;
        }
        else {
            c->Render();
        }
        it++;
    }
}

void ClientChunkManager::PushMeshPending(ClientChunk* c) {
    std::lock_guard<std::mutex> lock(meshIteratorMTX);
    meshIterationTransitionQueue.push(c);
}
void ClientChunkManager::PushRenderReady(ClientChunk* c) {
    std::lock_guard<std::mutex> lock(readyMutex);
    readyTransitionQueue.push(c);
}

void ClientChunkManager::MeshIteratorLoop() {
    while(ThreadRunning) {
        {
            std::lock_guard<std::mutex> lock(meshIteratorMTX);
            while(!meshIterationTransitionQueue.empty()) {
                MeshPendingSet.insert(meshIterationTransitionQueue.front());
                meshIterationTransitionQueue.pop();
            }
        }

        if(!MeshPendingSet.empty()) {
            for(auto it = MeshPendingSet.begin(); it != MeshPendingSet.end();) {
                ClientChunk* c = *it;
                
                c->GenerateMeshData();
                PushRenderReady(c);
                
                it = MeshPendingSet.erase(it);
            }
        } 
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}