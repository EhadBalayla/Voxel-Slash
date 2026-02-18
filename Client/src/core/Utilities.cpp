#include "Utilities.h"
#include "app.h"

#include "../World/Chunk.h"

bool IsChunkInRenderDistance(Chunk* c) {
    int ChunkX = App::Get()->m_Camera.ChunkCoordX;
    int ChunkY = App::Get()->m_Camera.ChunkCoordY;
    int ChunkZ = App::Get()->m_Camera.ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	return x <= App::Get()->RenderDistance && y <= App::Get()->RenderDistance && z <= App::Get()->RenderDistance;
}
bool ShouldLODRender(Chunk* c) {
	int ChunkX = App::Get()->m_Camera.ChunkCoordX;
    int ChunkY = App::Get()->m_Camera.ChunkCoordY;
    int ChunkZ = App::Get()->m_Camera.ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	if (x * 2 > App::Get()->RenderDistance - 1|| y * 2 > App::Get()->RenderDistance - 1 || z * 2 > App::Get()->RenderDistance - 1	) return true;
	return false;
}