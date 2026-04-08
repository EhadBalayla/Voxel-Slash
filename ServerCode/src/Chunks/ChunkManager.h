#pragma once
#include "ChunkProvider.h"
#include "ChunkGenerator.h"

#include "../Core Stuff/Block.h"

class LODParallelism;

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void UpdateChunks();

    ChunkProvider& GetChunkProvider();
    ChunkGenerator& GetChunkGenerator();

    bool IsUpdatingChunks = false;

    BlockType GetBlockAt(int x, int y, int z);
private:
    ChunkProvider m_ChunkProvider;
    ChunkGenerator m_ChunkGenerator;

    //extra threads for iterating over chunks at different stages
    bool ChunkIteratorsRunning = true;
    void chunksUpdaterLoop();
    std::mutex tempMTX;
    std::thread chunksUpdater;
    std::condition_variable updaterCV;
};