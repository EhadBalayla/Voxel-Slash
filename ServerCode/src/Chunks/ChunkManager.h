#pragma once
#include "ChunkProvider.h"
#include "ChunkGenerator.h"

#include "../Core Stuff/Block.h"
#include <queue>

#define GEN_THREADS_COUNT 6

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void UpdateChunks(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ);

    ChunkProvider& GetChunkProvider();
    ChunkGenerator& GetChunkGenerator();

    bool IsUpdatingChunks = false;

    BlockType GetBlockAt(int64_t x, int64_t y, int64_t z);
private:
    ChunkProvider m_ChunkProvider;
    ChunkGenerator m_ChunkGenerator;

    void PushGen(Chunk* c);

    //extra threads for iterating over chunks at different stages
    bool ThreadRunning = true;
    void chunksUpdaterLoop();
    int64_t CurrentChunkX, CurrentChunkY, CurrentChunkZ;
    std::mutex tempMTX;
    std::thread chunksUpdater;
    std::condition_variable updaterCV;

    void GenWorker();
    std::thread GenThread[GEN_THREADS_COUNT];
    std::mutex GenMTX;
    std::queue<Chunk*> GenQueue;
    std::condition_variable GenCV;
};