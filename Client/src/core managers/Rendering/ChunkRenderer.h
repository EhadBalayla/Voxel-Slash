#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>

#include "vk_mem_alloc.h"

#include <mutex>


struct UniformBuffer {
	std::vector<VkBuffer> Buffers;
	std::vector<VmaAllocation> BuffersAllocation;
	std::vector<void*> BuffersMapped;

	void Create(VkDeviceSize bufferSize, bool IsUniform /*true if uniform buffer, false if storage buffer*/);
	void Delete();
};

class ChunkRenderer {
public:
    void StartDescriptors();
	void EndDescriptors();

	void createDescriptorPool();

	VkDescriptorSetLayout GetChunksSetLayout() const;
	VkDescriptorSetLayout GetChunkMeshLayout() const;
	VkDescriptorSet GetChunksSet(int idx) const;
	VkPipelineLayout GetChunksPipelineLayout() const;

    //public helpers
	void SetViewProj(glm::mat4 view, glm::mat4 proj);
	void SetTrans(glm::mat4 trans);
private:
    //descriptor sets/layouts and uniform buffers
    UniformBuffer ChunkBuffer;
    std::vector<VkDescriptorSet> ChunkSets;
	VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout ChunkSetLayout;
	VkDescriptorSetLayout ChunkMeshLayout;
	VkPipelineLayout ChunksPipelineLayout;

    void CreateChunkSets();
};


struct MatricesBufferStruct {
    glm::mat4 proj;
    glm::mat4 view;
};