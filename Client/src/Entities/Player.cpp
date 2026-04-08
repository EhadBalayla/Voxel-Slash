#include "Player.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../core managers/app.h"
#include <GLFW/glfw3.h>

#include "../core/Utilities.h"

float MouseSensitivity = 0.5f;
bool pressedJump = false;

Player::Player() {
    aabb = {glm::vec3(-0.3f, 0.0f, -0.3f), glm::vec3(0.3f, 1.8f, 0.3f)};
    prefab.Load("Data/Prefabs/Player_Prefab.pfb", GApp->m_TempMod);

    acceleration = 1.0f;
    maxMovementSpeed = 10.0f;
}
void Player::Update(float DeltaTime) {
    UpdateChunksAroundPlayer();
    ProcessMovementInput();
    if(GApp->DoPhysics) MoveAndCollide(DeltaTime);

    if(velocity.x || velocity.z) {
        glm::vec3 forward = glm::normalize(glm::vec3(velocity.x, 0.0f, velocity.z));
        //Rotation = FInterp(Rotation, glm::degrees(glm::atan(forward.x, forward.z)), DeltaTime, 30.0f);
        Rotation = glm::degrees(glm::atan(forward.x, forward.z));
    }

    CameraPivotPosition = SmoothInterp(CameraPivotPosition, Position, DeltaTime, 10.0f);
}

void Player::ProcessMouseInput(float xoffset, float yoffset) {
    xoffset *= -MouseSensitivity;
    yoffset *= -MouseSensitivity;

    CamRotationYaw   += xoffset;
    CamRotationPitch += yoffset;

    if (CamRotationPitch > 89.0f)
        CamRotationPitch = 89.0f;
    if (CamRotationPitch < -89.0f)
        CamRotationPitch = -89.0f;
}
void Player::ProcessMovementInput() {
    glm::vec3 forward = GetCameraForwardVector();
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    glm::vec3 right = GetCameraRightVector();
    right.y = 0.0f;
    right = glm::normalize(right);

    if (glfwGetKey(GApp->m_Window.GetGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS)
        velocity += forward * acceleration;
    if (glfwGetKey(GApp->m_Window.GetGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS)
        velocity -= forward * acceleration;
    if (glfwGetKey(GApp->m_Window.GetGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS)
        velocity -= right * acceleration;
    if (glfwGetKey(GApp->m_Window.GetGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS)
        velocity += right * acceleration;

    if (glfwGetKey(GApp->m_Window.GetGLFWwindow(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        if(!pressedJump) {
            pressedJump = true;
            if(IsOnGround) velocity.y = 10.0f;
        }
    }
    else {
        if(pressedJump) {
            pressedJump = false;
        }
    }
}

glm::vec3 Player::GetCameraForwardVector() {
    glm::vec3 forward;
    forward.x = cos(glm::radians(CamRotationPitch)) * sin(glm::radians(CamRotationYaw));
    forward.y = sin(glm::radians(CamRotationPitch));
    forward.z = cos(glm::radians(CamRotationPitch)) * cos(glm::radians(CamRotationYaw));
    forward = glm::normalize(forward);
    return forward;
}
glm::vec3 Player::GetCameraRightVector() {
    glm::vec3 forward = GetCameraForwardVector();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::normalize(glm::cross(forward, up));
}

glm::mat4 Player::GetViewMatrix() {
    glm::vec3 pivot = CameraPivotPosition + glm::vec3(0.0f, CameraHeight, 0.0f);
    glm::quat rotation = glm::quat(glm::vec3(glm::radians(CamRotationPitch), glm::radians(CamRotationYaw), 0.0f));

    glm::vec3 offset(0.0f, 0.0f, CameraDistance);
    glm::vec3 rotatedOffset = rotation * offset;

    glm::vec3 cameraPos = pivot - rotatedOffset;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    return glm::lookAt(cameraPos, pivot, up);
}
void Player::UpdateChunksAroundPlayer() {
    if(GApp->LoadChunks)
    if(!GApp->m_World->GetChunkManager().IsUpdatingChunks) {
        int CurrentCoordX = static_cast<int>(std::floor(Position.x / 32.0f));
        int CurrentCoordY = static_cast<int>(std::floor(Position.y / 32.0f));
        int CurrentCoordZ = static_cast<int>(std::floor(Position.z / 32.0f));

        if(CurrentCoordX != ChunkCoordX || CurrentCoordY != ChunkCoordY || CurrentCoordZ != ChunkCoordZ) {
            ChunkCoordX = CurrentCoordX;
            ChunkCoordY = CurrentCoordY;
            ChunkCoordZ = CurrentCoordZ;

            GApp->m_World->GetChunkManager().UpdateChunks();
        }
    }
}