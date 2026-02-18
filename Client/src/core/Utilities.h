#pragma once
#include <glm/glm.hpp>


class Chunk;
bool IsChunkInRenderDistance(Chunk* c);
bool ShouldLODRender(Chunk* c);