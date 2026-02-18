#pragma once
#include "WorldManagers/Chunks/ChunkManager.h"

class World {
public:
    void UpdateWorld();
    void RenderWorld();

    ChunkManager& GetChunkManager();
private:
    ChunkManager m_ChunkManager;
};