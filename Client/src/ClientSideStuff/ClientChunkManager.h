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

    void AddNewChunk(glm::i64vec3 coords, void* data, bool HasAnything, int LOD);
    void RemoveChunk(glm::i64vec3 coords, int LOD);
    std::unordered_map<glm::i64vec3, ClientChunk*>& GetLoadedChunks(int LOD);
    ClientChunk* GetChunk(glm::i64vec3 coords, int LOD);

    void RenderChunks(); //simply put... renders all chunks that are render ready

private:
    std::unordered_map<glm::i64vec3, ClientChunk*> LoadedChunks[6];
    std::mutex LoadedChunksMTX[6];
    bool HasAllNeighbors(glm::i64vec3 coords, int LOD);

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