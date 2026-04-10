#pragma once
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <glm/glm.hpp>

#include "../core/ThreadPool.h"

namespace std {
	template<>
	struct hash<glm::i64vec3> {
        std::size_t operator()(const glm::i64vec3& k) const noexcept {
            std::size_t h1 = std::hash<int64_t>()(k.x);
            std::size_t h2 = std::hash<int64_t>()(k.y);
            std::size_t h3 = std::hash<int64_t>()(k.z);

            // Combine the hashes (boost-style)
            std::size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}

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