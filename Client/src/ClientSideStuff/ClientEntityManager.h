#pragma once
#include <glm/glm.hpp>

class ClientEntityManager {
public:
    glm::dvec3 playerPos = glm::dvec3(0.0);
    float playerRot = 0.0f;
};