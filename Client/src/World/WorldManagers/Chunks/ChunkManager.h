#pragma once
#include "ChunkProvider.h"
#include "ChunkGenerator.h"

#include "../../Block.h"

#include <queue>


#define GEN_THREADS_COUNT 6
#define MESH_THREADS_COUNT 4

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void Render();

    void UpdateChunks();

    ChunkProvider& GetChunkProvider();
    ChunkGenerator& GetChunkGenerator();

    bool IsUpdatingChunks = false;
    BlockType GetBlockAt(int x, int y, int z);
    void SetBlockNoUpdate(int x, int y, int z, int LOD, BlockType type);
private:
    bool ThreadRunning = true;
    ChunkProvider m_ChunkProvider;
    ChunkGenerator m_ChunkGenerator;

    void PushGen(Chunk* c);
    void PushMesh(Chunk* c);
    void PushReady(Chunk* c);

    void chunksUpdaterLoop();
    std::mutex tempMTX;
    std::thread chunksUpdater;
    std::condition_variable updaterCV;

    void GenWorker();
    std::thread GenThread[GEN_THREADS_COUNT];
    std::mutex GenMTX;
    std::queue<Chunk*> GenQueue;
    std::condition_variable GenCV;

    void MeshWorker();
    std::thread MeshThread[MESH_THREADS_COUNT];
    std::queue<Chunk*> MeshQueue;
    std::mutex MeshMTX;
    std::condition_variable MeshCV;


    
    std::mutex readyMutex;
    std::queue<Chunk*> readyTransitionQueue;
    std::vector<Chunk*> renderReadySet;

    std::mutex deletionMTX;
    std::queue<Chunk*> deletionQueue;
};