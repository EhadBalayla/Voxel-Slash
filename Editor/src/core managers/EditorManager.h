#pragma once
#include <vulkan/vulkan.h>
#include "imgui.h"
#include <vector>

class EditorManager {
public:
    void Init();
    void Render();
    void Finalize();
    void Terminate();
private:
    VkDescriptorPool editorPool;
    std::vector<ImTextureID> viewportBuffers;
};