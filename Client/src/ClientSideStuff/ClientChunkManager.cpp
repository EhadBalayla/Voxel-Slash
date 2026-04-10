#include "ClientChunkManager.h"
#include "ClientChunk.h"

#include "../core managers/app.h"

ClientChunkManager::ClientChunkManager() {
    MeshWorkerThread = std::thread(&ClientChunkManager::MeshWorkerLoop, this);
}
ClientChunkManager::~ClientChunkManager() {
    ThreadRunning = false;
    MeshCV.notify_all();
    MeshWorkerThread.join();
}

void ClientChunkManager::AddNewChunk(glm::i64vec3 coords, void* data, bool HasAnything) {
    if(LoadedChunks.find(coords) != LoadedChunks.end()) return;

    ClientChunk* c = new ClientChunk;
    c->ChunkX = coords.x;
    c->ChunkY = coords.y;
    c->ChunkZ = coords.z;
    c->LOD = 0;
    c->HasAnything = HasAnything;
    memcpy((void*)c->m_Blocks, data, 32*32*32);

    LoadedChunks[coords] = c;

    if(HasAllNeighbors(coords)) {
        PushMesh(c);
    }

    static glm::i64vec3 dirs[6] = {
        {1,0,0}, {-1,0,0},
        {0,1,0}, {0,-1,0},
        {0,0,1}, {0,0,-1}
    };

    for (auto d : dirs)
    {
        glm::i64vec3 n = coords + d;

        ClientChunk* ch = GetChunk(n);
        if (!ch) continue;

        if (!ch->IsMeshed && HasAllNeighbors(n))
            PushMesh(ch);
    }
}
std::unordered_map<glm::i64vec3, ClientChunk*>& ClientChunkManager::GetLoadedChunks() {
    return LoadedChunks;
}
ClientChunk* ClientChunkManager::GetChunk(glm::i64vec3 coords) {
    if(LoadedChunks.find(coords) != LoadedChunks.end()) return LoadedChunks[coords];
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
        }
        else {
            c->Render();
        }
        it++;
    }
}

bool ClientChunkManager::HasAllNeighbors(glm::i64vec3 coords) {
    static glm::i64vec3 offsets[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    for (auto of : offsets) {
        if (LoadedChunks.find(coords + of) == LoadedChunks.end()) return false;
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