#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

class Texture {
public:
    void Create(void* pixelData, int Width, int Height);
    void Delete();
private:
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
};