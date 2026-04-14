#include "ChunkBuffer.h"
#include "../app.h"

#include <stdexcept>

#include <cstring>

void ChunkBuffer::Update(void* facesData, size_t facesSize) {
    ChunkRenderer& renderer = GApp->m_ChunkRenderer;

    //allocating the regular mesh buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = static_cast<VkDeviceSize>(facesSize);
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if(vmaCreateBuffer(GRenderer->GetAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate mesh buffer of a chunk");
    }


    //copying the data to the GPU
    void* bData;
    vmaMapMemory(GRenderer->GetAllocator(), allocation, &bData);
    memcpy(bData, facesData, facesSize);
    vmaUnmapMemory(GRenderer->GetAllocator(), allocation);

    //making the descriptor pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 3;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = GContext->MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    
    if (vkCreateDescriptorPool(GRenderer->GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for storage buffer descriptor");
    }

    //making the descriptor set
    std::vector<VkDescriptorSetLayout> setLayouts(GContext->MAX_FRAMES_IN_FLIGHT, renderer.GetChunkMeshLayout());
    sets.resize(GContext->MAX_FRAMES_IN_FLIGHT);
    VkDescriptorSetAllocateInfo setsAllocInfo{};
    setsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setsAllocInfo.descriptorPool = pool;
    setsAllocInfo.descriptorSetCount = GContext->MAX_FRAMES_IN_FLIGHT;
    setsAllocInfo.pSetLayouts = setLayouts.data();
    if (vkAllocateDescriptorSets(GRenderer->GetDevice(), &setsAllocInfo, sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets for mesh buffer");
    }

    VkDescriptorBufferInfo pBufferInfo{};
    pBufferInfo.buffer = buffer;
    pBufferInfo.offset = 0;
    pBufferInfo.range = static_cast<VkDeviceSize>(facesSize);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &pBufferInfo;
    write.dstArrayElement = 0;

    for (int i = 0; i < GContext->MAX_FRAMES_IN_FLIGHT; i++) {
        write.dstSet = sets[i];

        vkUpdateDescriptorSets(GRenderer->GetDevice(), 1, &write, 0, nullptr);
    }
}
void ChunkBuffer::Delete() {
    vkDestroyDescriptorPool(GContext->GetDevice(), pool, nullptr);
    vmaDestroyBuffer(GContext->GetAllocator(), buffer, allocation);
}

VkBuffer ChunkBuffer::GetBuffer() const {
    return buffer;
}
VkDescriptorSet ChunkBuffer::GetDescriptorSet(int idx) const {
    return sets[idx];
}