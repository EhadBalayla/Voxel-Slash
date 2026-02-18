#pragma once
#include "ChunkProvider.h"
#include "ChunkGenerator.h"
#include "../../../core/LODParallelism.h"

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
    void PushGenChunk(Chunk* c);
    void PushDirtyChunk(Chunk* c);
    void PushUploadPending(Chunk* c);
    void PushReadyChunk(Chunk* c);
    void PushDeletionChunk(Chunk* c);
private:
    ChunkProvider m_ChunkProvider;
    ChunkGenerator m_ChunkGenerator;

    //extra threads for iterating over chunks at different stages
    bool ChunkIteratorsRunning = true;
    void chunksUpdaterLoop();
    std::mutex tempMTX;
    std::thread chunksUpdater;
    std::condition_variable updaterCV;


    void chunksMeshIteratorLoop(int LOD);
    std::mutex meshIteratorMTX[6];
    std::thread meshIterator[6];
    std::queue<Chunk*> meshIterationTransitionQueue[6];
    std::unordered_set<Chunk*> meshPendingSet[6];


    void chunksUploadIteratorLoop(int LOD);
    std::mutex uploadIteratorMTX[6];
    std::thread uploadIterator[6];
    std::queue<Chunk*> uploadIterationTransitionQueue[6];
    std::unordered_set<Chunk*> uploadPendingSet[6];


    void chunksDeletionIteratorLoop(int LOD);
    std::mutex deletionIteratorMTX[6];
    std::thread deletionIterator[6];
    std::queue<Chunk*> deletionIterationTransitionQueue[6];
    std::unordered_set<Chunk*> deletionPendingSet[6];
    
    std::mutex readyMutex;
    std::queue<Chunk*> readyTransitionQueue;
    std::unordered_set<Chunk*> renderReadySet;
};