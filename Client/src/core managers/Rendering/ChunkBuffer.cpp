#include "ChunkBuffer.h"
#include "../app.h"

#include "VulkanUtilities.h"
#include <stdexcept>

#include <cstring>

void ChunkBuffer::Update(void* verticiesData, size_t verticiesSize) {
    Renderer& renderer = GApp->m_Renderer;

    //allocating the regular mesh buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = static_cast<VkDeviceSize>(verticiesSize);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if(vmaCreateBuffer(renderer.GetAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate mesh buffer of a chunk");
    }


    //copying the data to the GPU
    void* bData;
    vmaMapMemory(renderer.GetAllocator(), allocation, &bData);
    memcpy(bData, verticiesData, verticiesSize);
    vmaUnmapMemory(renderer.GetAllocator(), allocation);
}
void ChunkBuffer::Delete() {
    Renderer& renderer = GApp->m_Renderer;

    vmaDestroyBuffer(renderer.GetAllocator(), buffer, allocation);
}

VkBuffer& ChunkBuffer::GetBuffer() {
    return buffer;
}