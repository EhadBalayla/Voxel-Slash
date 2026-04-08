#include "AABB.h"

AABB AABB::MovedTo(glm::vec3 pos) {
    return { min + pos, max + pos};
}

AABB blockHitbox = {glm::vec3(0.0f), glm::vec3(1.0f)};
bool Intersects(const AABB& a, const AABB& b) {
        return (a.max.x > b.min.x && a.min.x < b.max.x) &&
               (a.max.y > b.min.y && a.min.y < b.max.y) &&
               (a.max.z > b.min.z && a.min.z < b.max.z);
}