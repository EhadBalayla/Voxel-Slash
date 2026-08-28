#pragma once
#include <glm/glm.hpp>
#include <vector>

constexpr float SCameraHeight = 1.62f;
constexpr float SCameraDistance = 5.0f;

class ClientEntityManager {
public:
    uint64_t PlayerEntityID = 0;
    glm::dvec3 playerPos = glm::dvec3(0.0);
    float playerRot = 0.0f;

    glm::vec3 CameraPivotPosition = glm::vec3(0.0f);

    float CamRotationYaw = 0.0f;
    float CamRotationPitch = 0.0f;

    glm::mat4 GetViewMatrix();
    glm::vec3 GetCameraPosition();
    void InterpolateCamera(float DeltaTime);

    void ProcessMouseInput(float xoffset, float yoffset);
    float MouseSensitivity = 0.5f;

    std::vector<uint64_t> otherEntities;
    void RenderOtherEntities();
};