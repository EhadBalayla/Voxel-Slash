#include "Renderer.h"

#include <stdexcept>
#include <cstring>

#include "../app.h"

void ChunkRenderer::StartDescriptors() {
	createDescriptorPool();
    CreateChunkSets();
}
void ChunkRenderer::EndDescriptors() {
	vkDestroyPipelineLayout(GRenderer->GetDevice(), ChunksPipelineLayout, nullptr);

	vkDestroyDescriptorSetLayout(GRenderer->GetDevice(), ChunkSetLayout, nullptr);

	ChunkBuffer.Delete();
}

void ChunkRenderer::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1 * GContext->MAX_FRAMES_IN_FLIGHT;
	
	VkDescriptorPoolSize poolSize2{};
	poolSize2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize2.descriptorCount = 2 * GContext->MAX_FRAMES_IN_FLIGHT;
	
	VkDescriptorPoolSize poolSize3{};
	poolSize3.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize3.descriptorCount = 1 * GContext->MAX_FRAMES_IN_FLIGHT;
	
	uint32_t count = 3;
	VkDescriptorPoolSize poolSizes[] = { poolSize, poolSize2, poolSize3 };
	
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = count;
	createInfo.pPoolSizes = poolSizes;
	createInfo.maxSets = 2 * GContext->MAX_FRAMES_IN_FLIGHT;
	
	if (vkCreateDescriptorPool(GContext->GetDevice(), &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create descriptor pool");
	}
}



VkDescriptorSetLayout ChunkRenderer::GetChunksSetLayout() const {
	return ChunkSetLayout;
}
VkDescriptorSetLayout ChunkRenderer::GetChunkMeshLayout() const {
	return ChunkMeshLayout;
}
VkDescriptorSet ChunkRenderer::GetChunksSet(int idx) const {
	return ChunkSets[idx];
}
VkPipelineLayout ChunkRenderer::GetChunksPipelineLayout() const {
	return ChunksPipelineLayout;
}

void ChunkRenderer::SetViewProj(glm::mat4 view, glm::mat4 proj) {
	MatricesBufferStruct MBO = {proj, view};
	memcpy(ChunkBuffer.BuffersMapped[GContext->currentFrame], &MBO, sizeof(MBO));
}
void ChunkRenderer::SetTrans(glm::mat4 trans) {
	vkCmdPushConstants(GRenderer->GetFrameCommandBuffer(), ChunksPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &trans);
}

void ChunkRenderer::CreateChunkSets() {
    //allocate the uniform buffer
    VkDeviceSize MatricesB_SIZE = sizeof(MatricesBufferStruct);

    ChunkBuffer.Create(MatricesB_SIZE, true);
    
    //create the set layout
    VkDescriptorSetLayoutBinding MatricesBinding{};
    MatricesBinding.binding = 0;
    MatricesBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    MatricesBinding.descriptorCount = 1;
    MatricesBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorSetLayoutBinding TextureAtlasBinding{};
    TextureAtlasBinding.binding = 1;
    TextureAtlasBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    TextureAtlasBinding.descriptorCount = 1;
    TextureAtlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TextureAtlasBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding TextureMRSAtlasBinding{};
    TextureMRSAtlasBinding.binding = 2;
    TextureMRSAtlasBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    TextureMRSAtlasBinding.descriptorCount = 1;
    TextureMRSAtlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TextureMRSAtlasBinding.pImmutableSamplers = nullptr;

    uint32_t bindingCount = 3;
    VkDescriptorSetLayoutBinding bindings[] = {MatricesBinding, TextureAtlasBinding, TextureMRSAtlasBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = bindings;
    layoutInfo.bindingCount = bindingCount;

    if(vkCreateDescriptorSetLayout(GRenderer->GetDevice(), &layoutInfo, nullptr, &ChunkSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create chunks' descriptor set layout");
    }

	//create the set layout for mesh buffer
	VkDescriptorSetLayoutBinding meshBinding{};
	meshBinding.binding = 0;
	meshBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	meshBinding.descriptorCount = 1;
	meshBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	VkDescriptorSetLayoutCreateInfo meshLayoutInfo{};
	meshLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	meshLayoutInfo.pBindings = &meshBinding;
	meshLayoutInfo.bindingCount = 1;

	if (vkCreateDescriptorSetLayout(GRenderer->GetDevice(), &meshLayoutInfo, nullptr, &ChunkMeshLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create chunks mesh' descriptor set layout");
	}


	//create the pipeline layout
	VkDescriptorSetLayout ppSetLayouts[] = { ChunkSetLayout, ChunkMeshLayout };
	uint32_t setLayoutCounts = 2;

	VkPushConstantRange modelTransformRange{};
	modelTransformRange.offset = 0;
	modelTransformRange.size = sizeof(glm::mat4);
	modelTransformRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayoutCounts;
	pipelineLayoutInfo.pSetLayouts = ppSetLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &modelTransformRange;

	if(vkCreatePipelineLayout(GRenderer->GetDevice(), &pipelineLayoutInfo, nullptr, &ChunksPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create chunks pipeline layout");
	}



    //create the descriptor set/s
    std::vector<VkDescriptorSetLayout> setLayouts(GContext->MAX_FRAMES_IN_FLIGHT, ChunkSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pSetLayouts = setLayouts.data();
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = GContext->MAX_FRAMES_IN_FLIGHT;
    
    ChunkSets.resize(GContext->MAX_FRAMES_IN_FLIGHT);
    if(vkAllocateDescriptorSets(GRenderer->GetDevice(), &allocInfo, ChunkSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate the descriptor sets of the Chunks");
    }


    for(int i = 0; i < GContext->MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo MatricesBufferInfo{};
        MatricesBufferInfo.buffer = ChunkBuffer.Buffers[i];
        MatricesBufferInfo.offset = 0;
        MatricesBufferInfo.range = MatricesB_SIZE;

        VkDescriptorImageInfo AtlasInfo{};
        AtlasInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        AtlasInfo.imageView = GApp->m_TerrainAtlas.GetImageView();
        AtlasInfo.sampler = GRenderer->GetSampler();

		VkDescriptorImageInfo MRSAtlasInfo{};
        MRSAtlasInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        MRSAtlasInfo.imageView = GApp->m_TerrainMRSAtlas.GetImageView();
        MRSAtlasInfo.sampler = GRenderer->GetSampler();



        VkWriteDescriptorSet MatricesWrite{};
        MatricesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        MatricesWrite.dstSet = ChunkSets[i];
        MatricesWrite.dstBinding = 0;
        MatricesWrite.pBufferInfo = &MatricesBufferInfo;
        MatricesWrite.dstArrayElement = 0;
        MatricesWrite.descriptorCount = 1;
        MatricesWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkWriteDescriptorSet AtlasWrite{};
        AtlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        AtlasWrite.dstSet = ChunkSets[i];
        AtlasWrite.dstBinding = 1;
        AtlasWrite.pImageInfo = &AtlasInfo;
        AtlasWrite.dstArrayElement = 0;
        AtlasWrite.descriptorCount = 1;
        AtlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkWriteDescriptorSet MRSAtlasWrite{};
        MRSAtlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        MRSAtlasWrite.dstSet = ChunkSets[i];
        MRSAtlasWrite.dstBinding = 2;
        MRSAtlasWrite.pImageInfo = &MRSAtlasInfo;
        MRSAtlasWrite.dstArrayElement = 0;
        MRSAtlasWrite.descriptorCount = 1;
        MRSAtlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
 
        uint32_t writeCount = 3;
        VkWriteDescriptorSet descriptorWrites[] = {MatricesWrite, AtlasWrite, MRSAtlasWrite};
        vkUpdateDescriptorSets(GRenderer->GetDevice(), writeCount, descriptorWrites, 0, nullptr);
	}
}


void UniformBuffer::Create(VkDeviceSize bufferSize, bool IsUniform) {
	Buffers.resize(GContext->MAX_FRAMES_IN_FLIGHT);
	BuffersAllocation.resize(GContext->MAX_FRAMES_IN_FLIGHT);
	BuffersMapped.resize(GContext->MAX_FRAMES_IN_FLIGHT);

	//allocating the regular mesh buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = IsUniform ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;


	for(int i = 0; i < GContext->MAX_FRAMES_IN_FLIGHT; i++) {
		if(vmaCreateBuffer(GRenderer->GetAllocator(), &bufferInfo, &allocInfo, &Buffers[i], &BuffersAllocation[i], nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate mesh buffer of a chunk");
		}

    	vmaMapMemory(GRenderer->GetAllocator(), BuffersAllocation[i], &BuffersMapped[i]);
	}
}
void UniformBuffer::Delete() {
	for (int i = 0; i < GContext->MAX_FRAMES_IN_FLIGHT; i++) {
		vmaUnmapMemory(GContext->GetAllocator(), BuffersAllocation[i]);
		vmaDestroyBuffer(GContext->GetAllocator(), Buffers[i], BuffersAllocation[i]);
	}
}