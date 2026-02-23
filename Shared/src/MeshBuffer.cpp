#include "MeshBuffer.h"

#include <stdexcept>

void MeshBuffer::Create(void* verticies, size_t vertSize, void* indicies, size_t idxSize) {
    /*VkDeviceSize bufferSize = static_cast<VkDeviceSize>(verticiesSize) + static_cast<VkDeviceSize>(indiciesSize);
    indiciesOffset = verticiesSize;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if(vmaCreateBuffer(renderer.GetAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer for a mesh");
    }

    void* bData;
    vmaMapMemory(renderer.GetAllocator(), allocation, &bData);
    memcpy(bData, verticiesData, verticiesSize);
    memcpy((uint8_t*)bData + verticiesSize, indiciesData, indiciesSize);
    vmaUnmapMemory(renderer.GetAllocator(), allocation);*/
}
void MeshBuffer::Delete() {
    /*Renderer& renderer = GApp->m_Renderer;

    vmaDestroyBuffer(renderer.GetAllocator(), buffer, allocation);*/
}

VkBuffer MeshBuffer::GetBuffer() const {
    return buffer;
}