#include "ChunkManager.h"

#include "../Core Stuff/Chunk.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../Server.h"

#define SERVER_TEMP_LOD_COUNT 1 //defining a temporary LOD count for the server, until i'll add a setting for otherwise
#define SERVER_TEMP_RENDER_DISTANCE 2 //defining a temporary render distance for the server, until i'll add a setting for otherwise

ChunkManager::ChunkManager() : m_ChunkProvider(this) {
    chunksUpdater = std::thread(&ChunkManager::chunksUpdaterLoop, this);
    for(auto& t : GenThread) {
        t = std::thread(&ChunkManager::GenWorker, this);
    }

    std::cout << "Started the chunk manager" << std::endl;
}
ChunkManager::~ChunkManager() {
    ThreadRunning = false;
    updaterCV.notify_all();
    GenCV.notify_all();
    for(auto& t : GenThread) {
        t.join();
    }
    chunksUpdater.join();


    std::cout << "Ended the chunk manager" << std::endl;
}



void ChunkManager::UpdateChunks(int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ, int64_t PrevChunkX, int64_t PrevChunkY, int64_t PrevChunkZ) {
    IsUpdatingChunks = true;
    CurrentChunkX = ChunkX;
    CurrentChunkY = ChunkY;
    CurrentChunkZ = ChunkZ;
    LastChunkX = PrevChunkX;
    LastChunkY = PrevChunkY;
    LastChunkZ = PrevChunkZ;
    updaterCV.notify_one();
}



ChunkProvider& ChunkManager::GetChunkProvider() {
    return m_ChunkProvider;
}
ChunkGenerator& ChunkManager::GetChunkGenerator() {
    return m_ChunkGenerator;
}


void ChunkManager::PushGen(Chunk* c) {
    c->IsGenerating = true;
    c->IsGenerated = false;
    {
        std::lock_guard<std::mutex> lock(GenMTX);
        GenQueue.push(c);
    }
    GenCV.notify_one();
}


bool IsInRenderDistance(int64_t CenterX, int64_t CenterY, int64_t CenterZ, int64_t ChunkX, int64_t ChunkY, int64_t ChunkZ) {
    int64_t DifferenceX = std::abs(CenterX - ChunkX);
    int64_t DifferenceY = std::abs(CenterY - ChunkY);
    int64_t DifferenceZ = std::abs(CenterZ - ChunkZ);

    if(DifferenceX > SERVER_TEMP_RENDER_DISTANCE || DifferenceY > SERVER_TEMP_RENDER_DISTANCE || DifferenceZ > SERVER_TEMP_RENDER_DISTANCE) return false;
    return true;
}
void ChunkManager::chunksUpdaterLoop() {
    while(ThreadRunning) {
        int64_t LocalCenterX = 0;
        int64_t LocalCenterY = 0;
        int64_t LocalCenterZ = 0;

        int64_t LastCenterX = 0;
        int64_t LastCenterY = 0;
        int64_t LastCenterZ = 0;

        {
            std::unique_lock<std::mutex> lock(tempMTX);
            updaterCV.wait(lock, [this] {return IsUpdatingChunks || !ThreadRunning; });

            if(!ThreadRunning) break;

            LocalCenterX = CurrentChunkX;
            LocalCenterY = CurrentChunkY;
            LocalCenterZ = CurrentChunkZ;

            LastCenterX = LastChunkX;
            LastCenterY = LastChunkY;
            LastCenterZ = LastChunkZ;
        }
        for(int i = 0; i < SERVER_TEMP_LOD_COUNT; ++i) {

            int CenterX = LocalCenterX / GetLODSize(i);
            int CenterY = LocalCenterY / GetLODSize(i);
            int CenterZ = LocalCenterZ / GetLODSize(i);

            int PrevCenterX = LastCenterX / GetLODSize(i);
            int PrevCenterY = LastCenterY / GetLODSize(i);
            int PrevCenterZ = LastCenterZ / GetLODSize(i);

            for (int r = 0; r <= SERVER_TEMP_RENDER_DISTANCE; ++r) {

	    	    for (int dx = -r; dx <= r; ++dx) {
	    	        for (int dy = -r; dy <= r; ++dy) {
                        for(int dz = -r; dz <= r; ++dz) {
	    		            if (dx != r && dy != r && dz != r && dx != -r && dy != -r && dz != -r) continue;
                            //spawn new chunks or increment ref count of older chunks
	    		            Chunk* c = m_ChunkProvider.ProvideChunk(CenterX + dx, CenterY + dy, CenterZ + dz, i);
                            if(!c->IsGenerating && !c->IsGenerated) PushGen(c);
                            if(!IsInRenderDistance(PrevCenterX, PrevCenterY, PrevCenterZ, CenterX + dx, CenterY + dy, CenterZ + dz)) c->IncrementRefCount();

                            //gather old chunks to decrement ref count
                            c = m_ChunkProvider.ProvideChunk(PrevCenterX + dx, PrevCenterY + dy, PrevCenterZ + dz, i);
                            if(!IsInRenderDistance(CenterX, CenterY, CenterZ, PrevCenterX + dx, PrevCenterY + dy, PrevCenterZ + dz)) c->DecrementRefCount();


                            if(c->RefCount == 0) {
                                m_ChunkProvider.UnprovideChunk(PrevCenterX + dx, PrevCenterY + dy, PrevCenterZ + dz, i);
                            }
                        }
	    	        }
	    	    }
	        }
        }

        IsUpdatingChunks = false;
    }
}
void ChunkManager::GenWorker() {
    while(ThreadRunning) {
        Chunk* c;

        {
            std::unique_lock<std::mutex> lock(GenMTX);

            GenCV.wait(lock, [this] { return !GenQueue.empty() || !ThreadRunning; });

            if (!ThreadRunning) return;

            c = GenQueue.front();
            GenQueue.pop();
        }

        //if(!c->MarkedForDeletion) {
            m_ChunkGenerator.GenerateChunk(c);
            m_ChunkGenerator.ReplaceBlocks(c);
            m_ChunkGenerator.CarveCaves(c);
            c->IsGenerated = true;
        //}
        c->IsGenerating = false;


        GServer->m_NetworkManager.SendAllClientsASingleChunk(c);
    }
}


BlockType ChunkManager::GetBlockAt(int64_t x, int64_t y, int64_t z) {
    int64_t ChunkX = (int64_t)std::floor((double)x / Chunk_Length);
    int64_t ChunkY = (int64_t)std::floor((double)y / Chunk_Length);
    int64_t ChunkZ = (int64_t)std::floor((double)z / Chunk_Length);

    int LocalX = x - ChunkX * Chunk_Length;
    int LocalY = y - ChunkY * Chunk_Length;
    int LocalZ = z - ChunkZ * Chunk_Length;

    return m_ChunkProvider.ProvideChunk(ChunkX, ChunkY, ChunkZ, 0)->m_Blocks[IndexAt(LocalX, LocalY, LocalZ)];
}