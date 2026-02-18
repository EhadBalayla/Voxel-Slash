#pragma once
#include <glm/glm.hpp>

struct Plane {
    glm::vec3 normal;
    float d;
};
struct Frustum {
    Plane planes[6];
};
Frustum ExtractFrustum(const glm::mat4& projView);
bool ChunkInFrustum(const Frustum& f, const glm::vec3& min, const glm::vec3& max);