#pragma once
#include <unordered_map>

#include <glm/glm.hpp>


class ClientChunk;
class ClientChunkManager {
public:
    std::unordered_map<glm::ivec3, ClientChunk*> LoadedChunks;
private:

};