#include "ChunkManager.h"

#include "../Core Stuff/Chunk.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#define SERVER_TEMP_LOD_COUNT 1 //defining a temporary LOD count for the server, until i'll add a setting for otherwise
#define SERVER_TEMP_RENDER_DISTANCE 2 //defining a temporary render distance for the server, until i'll add a setting for otherwise

ChunkManager::ChunkManager() : m_ChunkProvider(this) {
    chunksUpdater = std::thread(&ChunkManager::chunksUpdaterLoop, this);
    /*LODParallels = new LODParallelism*[GApp->MaxLODLevel];
    for(int i = 0; i < GApp->MaxLODLevel; i++) {
        LODParallels[i] = new LODParallelism(i, this);
    }*/

    std::cout << "Started the chunk manager" << std::endl;
}
ChunkManager::~ChunkManager() {
    ChunkIteratorsRunning = false;
    updaterCV.notify_all();

    chunksUpdater.join();
    /*for(int i = 0; i < GApp->MaxLODLevel; i++) {
        delete LODParallels[i];
    }
    delete[] LODParallels;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_ChunkProvider.DeleteAllChunks();*/

    std::cout << "Ended the chunk manager" << std::endl;
}



void ChunkManager::UpdateChunks() {
    IsUpdatingChunks = true;
    updaterCV.notify_one();
}



ChunkProvider& ChunkManager::GetChunkProvider() {
    return m_ChunkProvider;
}
ChunkGenerator& ChunkManager::GetChunkGenerator() {
    return m_ChunkGenerator;
}



void ChunkManager::chunksUpdaterLoop() {
    while(ChunkIteratorsRunning) {
        {
            std::unique_lock<std::mutex> lock(tempMTX);
            updaterCV.wait(lock, [this] {return IsUpdatingChunks || !ChunkIteratorsRunning; });

            if(!ChunkIteratorsRunning) break;
        }
        for(int i = 0; i < SERVER_TEMP_LOD_COUNT; i++) {

            int CenterX = 0; //GApp->m_Player->ChunkCoordX / GetLODSize(i);
            int CenterY = 0; //GApp->m_Player->ChunkCoordY / GetLODSize(i);
            int CenterZ = 0; //GApp->m_Player->ChunkCoordZ / GetLODSize(i);

            for (int r = 0; r <= SERVER_TEMP_RENDER_DISTANCE; r++) {

	    	    for (int dx = -r; dx <= r; dx++) {
	    	        for (int dy = -r; dy <= r; dy++) {
                        for(int dz = -r; dz <= r; dz++) {

	    		            if (dx != r && dy != r && dz != r && dx != -r && dy != -r && dz != -r) continue;
	    		            m_ChunkProvider.ProvideChunk(CenterX + dx, CenterY + dy, CenterZ + dz, i);
                        }
	    	        }
	    	    }
	        }
        }

        IsUpdatingChunks = false;
    }
}


BlockType ChunkManager::GetBlockAt(int x, int y, int z) {
    int ChunkX = (int)std::floor((double)x / Chunk_Length);
    int ChunkY = (int)std::floor((double)y / Chunk_Length);
    int ChunkZ = (int)std::floor((double)z / Chunk_Length);

    int LocalX = x - ChunkX * Chunk_Length;
    int LocalY = y - ChunkY * Chunk_Length;
    int LocalZ = z - ChunkZ * Chunk_Length;

    return m_ChunkProvider.ProvideChunk(ChunkX, ChunkY, ChunkZ, 0)->m_Blocks[IndexAt(LocalX, LocalY, LocalZ)];
}