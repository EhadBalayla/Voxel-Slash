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

void ClientEntityManager::ProcessMouseInput(float xoffset, float yoffset) {
    xoffset *= -MouseSensitivity;
    yoffset *= -MouseSensitivity;

    CamRotationYaw   += xoffset;
    CamRotationPitch += yoffset;

    if (CamRotationPitch > 89.0f)
        CamRotationPitch = 89.0f;
    if (CamRotationPitch < -89.0f)
        CamRotationPitch = -89.0f;
}

#include "../core managers/app.h"
#include "Prefab.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
void ClientEntityManager::RenderOtherEntities() {
    if(otherEntities.size() > 0) {
        for(auto entity : otherEntities) {
            glm::mat4 mat = glm::mat4(1.0f);
            mat = glm::translate(mat, (glm::vec3)entity.pos);
            mat = glm::rotate(mat, glm::radians(entity.rot), glm::vec3(0.0f, 1.0f, 0.0f));

            GApp->m_BoxOutlineShader.Bind();
            GApp->m_ChunkRenderer.SetTrans(mat);
            VkDescriptorSet sets[] = { GApp->m_ChunkRenderer.GetChunksSet(GContext->currentFrame) };
            vkCmdBindDescriptorSets(GRenderer->GetFrameCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, GApp->m_ChunkRenderer.GetChunksPipelineLayout(), 0, 1, sets, 0, nullptr);
            vkCmdDraw(GRenderer->GetFrameCommandBuffer(), 24, 1, 0, 0);

            GApp->m_SkeletalMeshShader.Bind();
            Prefab* pfb = GApp->m_TempMod->GetAllPrefabs()["Player_Prefab"];
            pfb->Render(GRenderer->GetFrameCommandBuffer(), GApp->m_ChunkRenderer.GetChunksPipelineLayout(), mat);
        }
    }
}