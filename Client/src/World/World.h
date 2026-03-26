#pragma once
#include "WorldManagers/Chunks/ChunkManager.h"
#include "WorldManagers/TickManager.h"
#include "WorldManagers/EntityManager.h"

class World {
public:
    void UpdateWorld();
    void RenderWorld();

    ChunkManager& GetChunkManager();
private:
    ChunkManager m_ChunkManager;
    TickManager m_TickManager;
    EntityManager m_EntityManager;
};