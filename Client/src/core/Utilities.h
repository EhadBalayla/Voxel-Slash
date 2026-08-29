#pragma once
#include <glm/glm.hpp>


class Chunk;
class ClientChunk;
bool IsChunkInRenderDistance(Chunk* c);
bool IsChunkInBufferDistance(Chunk* c);
bool ShouldLODRender(ClientChunk* c);
glm::vec3 SmoothInterp(const glm::vec3& current, const glm::vec3& target, float deltaTime, float speed);
float FInterp(float current, float target, float deltaTime, float speed);