#pragma once
#include "vk_mem_alloc.h"
#include <vector>

class ChunkBuffer {
public:
    void Update(void* facesData, size_t facesSize);
    void Delete();

    VkBuffer GetBuffer() const;
    VkDescriptorSet GetDescriptorSet(int idx) const;
private:
    VmaAllocation allocation;
    VkBuffer buffer;

    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets;

    bool IsAllocated = false;
};