#pragma once
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <queue>

#include <glm/glm.hpp>

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

    void AddNewChunk(glm::i64vec3 coords, void* data);

    void RenderChunks(); //simply put... renders all chunks that are render ready
private:
    std::unordered_map<glm::i64vec3, ClientChunk*> LoadedChunks;    

    void PushMeshPending(ClientChunk* c);
    void PushRenderReady(ClientChunk* c);

    bool ThreadRunning = true;
    void MeshIteratorLoop();
    std::unordered_set<ClientChunk*> MeshPendingSet;
    std::mutex meshIteratorMTX;
    std::thread meshIterator;
    std::queue<ClientChunk*> meshIterationTransitionQueue;

    std::unordered_set<ClientChunk*> RenderReadySet;
    std::mutex readyMutex;
    std::queue<ClientChunk*> readyTransitionQueue;
};