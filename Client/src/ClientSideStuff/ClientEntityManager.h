#pragma once
#include <glm/glm.hpp>

constexpr float SCameraHeight = 1.62f;
constexpr float SCameraDistance = 5.0f;

class ClientEntityManager {
public:
    glm::dvec3 playerPos = glm::dvec3(0.0);
    float playerRot = 0.0f;

    glm::vec3 CameraPivotPosition = glm::vec3(0.0f);

    float CamRotationYaw = 0.0f;
    float CamRotationPitch = 0.0f;

    glm::mat4 GetViewMatrix();
    void InterpolateCamera(float DeltaTime);
};