#include <vulkan/vulkan.h>


class DebugUI {
public:
    void Init();
    void RenderDebugUI();
    void RenderMenuDebugUI();
    void Terminate();

private:
    void CreateVulkanResources();

    VkDescriptorPool debugUIPool;
};