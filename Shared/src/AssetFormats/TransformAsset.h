#include "Asset.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

//a temporary abstract that inherits from asset but aims to be a base for spatial assets
class TransformAsset : public Asset{
public:
    virtual void Render(VkCommandBuffer cmd, VkPipelineLayout layout, glm::mat4 mtx) = 0;
};