#pragma once
#include <glm/glm.hpp>


class Chunk;
bool IsChunkInRenderDistance(Chunk* c);
bool IsChunkInBufferDistance(Chunk* c);
bool ShouldLODRender(Chunk* c);
glm::vec3 SmoothInterp(const glm::vec3& current, const glm::vec3& target, float deltaTime, float speed);