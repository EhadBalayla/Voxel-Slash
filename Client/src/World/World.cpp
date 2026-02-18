#include "World.h"

void World::UpdateWorld() {
    
}
void World::RenderWorld() {
    m_ChunkManager.Render();
}


ChunkManager& World::GetChunkManager() {
    return m_ChunkManager;
}