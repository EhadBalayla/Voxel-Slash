#include "Utilities.h"
#include "../core managers/app.h"

#include "../World/Chunk.h"

bool IsChunkInRenderDistance(Chunk* c) {
    int ChunkX = GApp->m_Player->ChunkCoordX;
    int ChunkY = GApp->m_Player->ChunkCoordY;
    int ChunkZ = GApp->m_Player->ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	return x <= GApp->RenderDistance && y <= GApp->RenderDistance && z <= GApp->RenderDistance;
}
bool IsChunkInBufferDistance(Chunk* c) {
	int ChunkX = GApp->m_Player->ChunkCoordX;
    int ChunkY = GApp->m_Player->ChunkCoordY;
    int ChunkZ = GApp->m_Player->ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	return x <= GApp->RenderDistance + 1 && y <= GApp->RenderDistance + 1 && z <= GApp->RenderDistance + 1;
}
bool ShouldLODRender(Chunk* c) {
	int ChunkX = GApp->m_Player->ChunkCoordX;
    int ChunkY = GApp->m_Player->ChunkCoordY;
    int ChunkZ = GApp->m_Player->ChunkCoordZ;

	int LOD_X = ChunkX / GetLODSize(c->LOD);
    int LOD_Y = ChunkY / GetLODSize(c->LOD);
	int LOD_Z = ChunkZ / GetLODSize(c->LOD);

	int x = std::abs(c->ChunkX - LOD_X);
    int y = std::abs(c->ChunkY - LOD_Y);
	int z = std::abs(c->ChunkZ - LOD_Z);

	if (x * 2 > GApp->RenderDistance - 1 || y * 2 > GApp->RenderDistance - 1 || z * 2 > GApp->RenderDistance - 1) return true;
	return false;
}
glm::vec3 SmoothInterp(const glm::vec3& current, const glm::vec3& target, float deltaTime, float speed) {
    float alpha = 1.0f - expf(-speed * deltaTime);
    return current + (target - current) * alpha;
}
float FInterp(float current, float target, float deltaTime, float speed) {
	float alpha = 1.0f - expf(-speed * deltaTime);
    return current + (target - current) * alpha;
}

