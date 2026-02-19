#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB MovedTo(glm::vec3 pos);
};

extern AABB blockHitbox;
bool Intersects(const AABB& a, const AABB& b);