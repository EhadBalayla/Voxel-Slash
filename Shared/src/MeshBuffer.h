#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class MeshBuffer {
public:
    void Create(void* verticies, size_t vertSize, void* indicies, size_t idxSize);
    void Delete();

    VkBuffer GetBuffer() const;

    VkDeviceSize indiciesOffset;
private:
    VkBuffer buffer;
    VmaAllocation allocation;
};