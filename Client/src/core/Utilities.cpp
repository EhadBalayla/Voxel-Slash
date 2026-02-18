#include "Utilities.h"
#include "app.h"

#include "../World/Chunk.h"

bool IsChunkInRenderDistance(Chunk* c) {
    int ChunkX = GApp->m_Camera.ChunkCoordX;
    int ChunkY = GApp->m_Camera.ChunkCoordY;
    int ChunkZ = GApp->m_Camera.ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	return x <= GApp->RenderDistance && y <= GApp->RenderDistance && z <= GApp->RenderDistance;
}
bool ShouldLODRender(Chunk* c) {
	int ChunkX = GApp->m_Camera.ChunkCoordX;
    int ChunkY = GApp->m_Camera.ChunkCoordY;
    int ChunkZ = GApp->m_Camera.ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	if (x * 2 > GApp->RenderDistance - 1|| y * 2 > GApp->RenderDistance - 1 || z * 2 > GApp->RenderDistance - 1	) return true;
	return false;
}