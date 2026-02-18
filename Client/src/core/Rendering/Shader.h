#pragma once
#include <string>
#include "glm/glm.hpp"

#include <vulkan/vulkan.h>

enum class PipelineType {
    Chunk,
    DebugChunkBorder
};

class Shader
{
public:
    void LoadShader(const char* vertexPath, const char* fragmentPath, PipelineType type);
    void UnloadShader();

    void Bind();
private:
    VkPipeline graphicsPipeline;
};