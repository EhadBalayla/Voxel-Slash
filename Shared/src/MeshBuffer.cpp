#include "MeshBuffer.h"
#include "Context.h"

#include <stdexcept>

void MeshBuffer::Create(void* verticies, size_t vertSize, void* indicies, size_t idxSize) {
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(vertSize) + static_cast<VkDeviceSize>(idxSize);
    indiciesOffset = vertSize;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if(vmaCreateBuffer(GContext->GetAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer for a mesh");
    }

    void* bData;
    vmaMapMemory(GContext->GetAllocator(), allocation, &bData);
    memcpy(bData, verticies, vertSize);
    memcpy((uint8_t*)bData + vertSize, indicies, idxSize);
    vmaUnmapMemory(GContext->GetAllocator(), allocation);
}
void MeshBuffer::Delete() {
    vmaDestroyBuffer(GContext->GetAllocator(), buffer, allocation);
}

VkBuffer MeshBuffer::GetBuffer() const {
    return buffer;
}