#include "ClientEntityManager.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../core/Utilities.h"

glm::mat4 ClientEntityManager::GetViewMatrix() {
    glm::vec3 pivot = CameraPivotPosition + glm::vec3(0.0f, SCameraHeight, 0.0f);
    glm::quat rotation = glm::quat(glm::vec3(glm::radians(CamRotationPitch), glm::radians(CamRotationYaw), 0.0f));

    glm::vec3 offset(0.0f, 0.0f, SCameraDistance);
    glm::vec3 rotatedOffset = rotation * offset;

    glm::vec3 cameraPos = pivot - rotatedOffset;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    return glm::lookAt(cameraPos, pivot, up);
}
glm::vec3 ClientEntityManager::GetCameraPosition() {
    glm::vec3 pivot = CameraPivotPosition + glm::vec3(0.0f, SCameraHeight, 0.0f);
    glm::quat rotation = glm::quat(glm::vec3(glm::radians(CamRotationPitch), glm::radians(CamRotationYaw), 0.0f));

    glm::vec3 offset(0.0f, 0.0f, SCameraDistance);
    glm::vec3 rotatedOffset = rotation * offset;

    return pivot - rotatedOffset;
}
void ClientEntityManager::InterpolateCamera(float DeltaTime) {
    CameraPivotPosition = SmoothInterp(CameraPivotPosition, playerPos, DeltaTime, 10.0f);
}