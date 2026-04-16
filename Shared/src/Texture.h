#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

class Texture {
public:
    void LoadFromFile(const char* path, bool UNorm = false);
    void Create(void* pixelData, int Width, int Height, bool UNorm = false);
    void Delete();

    VkImageView GetImageView() const;
private:
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
};