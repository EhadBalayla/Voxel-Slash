#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class FullscreenQuad
{
public:
    void CreateFullscreenQuad();
    void DeleteFullscreenQuad();

    void SetTexture();
    void Draw();
private:
    void CreateDescriptorSets();
    void CreatePipeline();

    VkDescriptorPool fullscreenPool;
    VkDescriptorSetLayout fullscreenSetLayout;
    std::vector<VkDescriptorSet> fullscreenSets;

    VkPipelineLayout fullscreenPipelineLayout;
    VkPipeline graphicsPipeline;
};