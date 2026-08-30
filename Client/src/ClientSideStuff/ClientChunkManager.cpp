#include "ClientChunkManager.h"
#include "ClientChunk.h"

#include "../core managers/app.h"

#include "../core/Utilities.h"

ClientChunkManager::ClientChunkManager() {
    MeshWorkerThread = std::thread(&ClientChunkManager::MeshWorkerLoop, this);
}
ClientChunkManager::~ClientChunkManager() {
    ThreadRunning = false;
    MeshCV.notify_all();
    MeshWorkerThread.join();
}

void ClientChunkManager::AddNewChunk(glm::i64vec3 coords, void* data, bool HasAnything, int LOD) {
    auto& MAP = GetLoadedChunks(LOD);
    {
        std::lock_guard<std::mutex> lock(LoadedChunksMTX[LOD]);
        if(MAP.find(coords) != MAP.end()) return;
    }

    ClientChunk* c = new ClientChunk;
    c->ChunkX = coords.x;
    c->ChunkY = coords.y;
    c->ChunkZ = coords.z;
    c->LOD = LOD;
    c->HasAnything = HasAnything;
    memcpy((void*)c->m_Blocks, data, 32*32*32);

    {
        std::lock_guard<std::mutex> lock(LoadedChunksMTX[LOD]);
        MAP[coords] = c;
    }

    if(HasAllNeighbors(coords, LOD)) {
        PushMesh(c);
    }

    static glm::i64vec3 dirs[6] = {
        {1,0,0}, {-1,0,0},
        {0,1,0}, {0,-1,0},
        {0,0,1}, {0,0,-1}
    };

    for (auto& d : dirs)
    {
        glm::i64vec3 n = coords + d;

        ClientChunk* ch = GetChunk(n, LOD);
        if (!ch) continue;

        if (!ch->IsMeshed && HasAllNeighbors(n, LOD))
            PushMesh(ch);
    }
}
void ClientChunkManager::RemoveChunk(glm::i64vec3 coords, int LOD) {
    auto& MAP = GetLoadedChunks(LOD);

    ClientChunk* c;
    {
        std::lock_guard<std::mutex> lock(LoadedChunksMTX[LOD]);
        c = MAP[coords];
    }

    if(!c->IsMeshed) {
        MAP.erase(coords);
        return;
    }
    c->IsMarkedForDeletion = true;
    {
        MAP.erase(coords);
    }
}
std::unordered_map<glm::i64vec3, ClientChunk*>& ClientChunkManager::GetLoadedChunks(int LOD) {
    return LoadedChunks[LOD];
}
ClientChunk* ClientChunkManager::GetChunk(glm::i64vec3 coords, int LOD) {
    auto& MAP = GetLoadedChunks(LOD);
    std::lock_guard<std::mutex> lock(LoadedChunksMTX[LOD]);
    if(MAP.find(coords) != MAP.end()) return MAP[coords];
    return nullptr;
}

void ClientChunkManager::RenderChunks() {
    std::queue<ClientChunk*> temp;
    {
        std::lock_guard<std::mutex> lock(RenderReadyMTX);
        std::swap(temp, RenderReadyQueue);
    }
    while (!temp.empty()) {
        RenderReadyChunks.push_back(temp.front());
        temp.pop();
    }

    GApp->m_OpaqueShader.Bind();
    for(auto it = RenderReadyChunks.begin(); it != RenderReadyChunks.end();) {
        ClientChunk* c = *it;

        if(!c->IsRenderReady) {
            c->IsRenderReady = true;
            c->UploadMeshData();
            ++it;
        }
        else {  
            if(c->IsMarkedForDeletion) {
                it = RenderReadyChunks.erase(it);
                continue;
                //temporarily erase chunk without erasing RAM or freeing VRAM cause i am NOT dealing with Vulkan syncronization issues at THIS moment in time okay no fucking way.
            }
            ++it;
            if(c->LOD > 0 && !ShouldLODRender(c)) continue;

            c->Render();
        }
    }
}

bool ClientChunkManager::HasAllNeighbors(glm::i64vec3 coords, int LOD) {
    static glm::i64vec3 offsets[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    auto& MAP = GetLoadedChunks(LOD);
    std::lock_guard<std::mutex> lock(LoadedChunksMTX[LOD]);
    for (auto& of : offsets) {
        if (MAP.find(coords + of) == MAP.end()) return false;
    }
    return true;
}


void ClientChunkManager::PushMesh(ClientChunk* c) {
    c->IsMeshed = true;
    {
        std::unique_lock<std::mutex> lock(MeshMTX);
        MeshQueue.push(c);
    }
    MeshCV.notify_one();
}
void ClientChunkManager::PushRenderReady(ClientChunk* c) {
    std::lock_guard<std::mutex> lock(RenderReadyMTX);
    RenderReadyQueue.push(c);
}

void ClientChunkManager::MeshWorkerLoop() {
    while(ThreadRunning) {
        ClientChunk* c;

        {
            std::unique_lock<std::mutex> lock(MeshMTX);

            MeshCV.wait(lock, [this] { return !MeshQueue.empty() || !ThreadRunning; });

            if (!ThreadRunning) return;

            c = MeshQueue.front();
            MeshQueue.pop();
        }

        c->GenerateMeshData();
        PushRenderReady(c);
    }
}