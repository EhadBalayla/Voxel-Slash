#pragma once
#include "ChunkProvider.h"
#include "ChunkGenerator.h"

class LODParallelism;

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void Update();
    void Render();

    void UpdateChunks();

    ChunkProvider& GetChunkProvider();
    ChunkGenerator& GetChunkGenerator();

    bool IsUpdatingChunks = false;
    void PushReadyChunk(Chunk* c);

    LODParallelism** LODParallels;
private:
    ChunkProvider m_ChunkProvider;
    ChunkGenerator m_ChunkGenerator;

    //extra threads for iterating over chunks at different stages
    bool ChunkIteratorsRunning = true;
    void chunksUpdaterLoop();
    std::mutex tempMTX;
    std::thread chunksUpdater;
    std::condition_variable updaterCV;

    
    std::mutex readyMutex;
    std::queue<Chunk*> readyTransitionQueue;
    std::unordered_set<Chunk*> renderReadySet;
};