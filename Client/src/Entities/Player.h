#pragma once
#include "../core/Entity.h"

constexpr float CameraHeight = 1.62f;
constexpr float CameraDistance = 5.0f;

class Player : public Entity {
public:
    Player();
    void Update(float DeltaTime) override;

    void ProcessMouseInput(float xoffset, float yoffset);
    void ProcessMovementInput();

    glm::vec3 GetCameraForwardVector();
    glm::vec3 GetCameraRightVector();

    glm::mat4 GetViewMatrix();
    glm::vec3 GetCameraPosition();
    void UpdateChunksAroundPlayer();

    int ChunkCoordX = 0;
    int ChunkCoordY = 0;
    int ChunkCoordZ = 0;
private:
    glm::vec3 CameraPivotPosition = glm::vec3(0.0f);

    float CamRotationYaw = 0.0f;
    float CamRotationPitch = 0.0f;
};