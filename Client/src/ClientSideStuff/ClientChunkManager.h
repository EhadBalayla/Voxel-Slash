#pragma once
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <glm/glm.hpp>

#include "Chunks/ChunkProvider.h"

class ClientChunk;
class ClientChunkManager {
public:
    ClientChunkManager();
    ~ClientChunkManager();

    void AddNewChunk(glm::i64vec3 coords, void* data, bool HasAnything);
    std::unordered_map<glm::i64vec3, ClientChunk*>& GetLoadedChunks();
    ClientChunk* GetChunk(glm::i64vec3 coords);

    void RenderChunks(); //simply put... renders all chunks that are render ready

private:
    std::unordered_map<glm::i64vec3, ClientChunk*> LoadedChunks;
    bool HasAllNeighbors(glm::i64vec3 coords);

    void PushMesh(ClientChunk* c);
    void PushRenderReady(ClientChunk* c);

    bool ThreadRunning = true;
    void MeshWorkerLoop();
    std::thread MeshWorkerThread;
    std::queue<ClientChunk*> MeshQueue;
    std::mutex MeshMTX;
    std::condition_variable MeshCV;

    std::queue<ClientChunk*> RenderReadyQueue;
    std::mutex RenderReadyMTX;
    std::vector<ClientChunk*> RenderReadyChunks;
};