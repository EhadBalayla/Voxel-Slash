#pragma once
#include "ThreadPool.h"
#include <unordered_set>

class Chunk;
class ChunkManager;

class LODParallelism {
public:
    LODParallelism(int LOD, ChunkManager* manager);
    ~LODParallelism();

    void GenerateChunk(Chunk* c);
    void PushDirtyChunk(Chunk* c);
    void PushUploadPending(Chunk* c);
    void PushDeletionChunk(Chunk* c);
private:
    int LOD = 0;
    ChunkManager* owningChunkManager = nullptr;
    bool ChunkIteratorsRunning = true;

    void chunksMeshIteratorLoop();
    std::mutex meshIteratorMTX;
    std::thread meshIterator;
    std::queue<Chunk*> meshIterationTransitionQueue;
    std::unordered_set<Chunk*> meshPendingSet;


    void chunksUploadIteratorLoop();
    std::mutex uploadIteratorMTX;
    std::thread uploadIterator;
    std::queue<Chunk*> uploadIterationTransitionQueue;
    std::unordered_set<Chunk*> uploadPendingSet;


    void chunksDeletionIteratorLoop();
    std::mutex deletionIteratorMTX;
    std::thread deletionIterator;
    std::queue<Chunk*> deletionIterationTransitionQueue;
    std::unordered_set<Chunk*> deletionPendingSet;

    ThreadPool GenPool;
    ThreadPool MeshPool;
    ThreadPool UploadPool;

    void MeshChunk(Chunk* c);
    void UploadChunk(Chunk* c);
};