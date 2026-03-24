#pragma once
#include <string>
#include "glm/glm.hpp"

#include <vulkan/vulkan.h>

enum class PipelineType {
    Chunk,
    BoxOutline,
    SkeletalMesh,
    StaticMesh,
    UIShader,
};

class Shader
{
public:
    void LoadShader(const char* vertexPath, const char* fragmentPath, PipelineType type);
    void UnloadShader();

    void Bind();

    VkPipeline* GetPipeline();
private:
    VkPipeline graphicsPipeline;
};