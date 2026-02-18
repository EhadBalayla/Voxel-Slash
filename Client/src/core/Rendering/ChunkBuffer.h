#pragma once
#include "vk_mem_alloc.h"

class ChunkBuffer {
public:
    void Update(void* verticiesData, size_t verticiesSize, void* indiciesData, size_t indiciesSize);
    void Delete();

    VkBuffer& GetBuffer();
    VkDeviceSize indiciesOffset;
private:
	//SubAllocation allocation;
    VmaAllocation allocation;
    VkBuffer buffer;

    bool IsAllocated = false;
};