#define VMA_IMPLEMENTATION
#include "Renderer.h"

#include <stdexcept>
#include <cstring>

#include "../app.h"

#include "VulkanUtilities.h"

void Renderer::Init() {
    createCommandPool();
    createCommandBuffers();
    createDescriptorPool();
    createTextureSampler();
	createAllocator();
}
void Renderer::StartDescriptors() {
    CreateChunkSets();
}
void Renderer::EndDescriptors() {
	vkDestroyPipelineLayout(device, ChunksPipelineLayout, nullptr);

	vkDestroyDescriptorSetLayout(device, ChunkSetLayout, nullptr);

	ChunkBuffer.Delete();
}
void Renderer::Terminate() {
	//destroy allocator
	vmaDestroyAllocator(allocator);

	//destroy texture sampler
	vkDestroySampler(device, sampler, nullptr);

	//destroy descriptor pool
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	//destroy command buffers
	vkFreeCommandBuffers(device, commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers.data());

	//destroy command pool
	vkDestroyCommandPool(device, commandPool, nullptr);
}


VkInstance& Renderer::GetInstance() {
	return instance;
}
VkPhysicalDevice& Renderer::GetPhysicalDevice() {
	return physicalDevice;
}
VkDevice& Renderer::GetDevice() {
	return device;
}
VkSurfaceKHR& Renderer::GetSurface() {
	return surface;
}
VkCommandPool& Renderer::GetCommandPool() {
	return commandPool;
}
VkQueue& Renderer::GetGraphicsQueue() {
	return graphicsQueue;
}
VkQueue& Renderer::GetPresentQueue() {
	return presentQueue;
}
uint32_t Renderer::GetGraphicsFamilyIndex() {
	return graphicsFamilyIndex;
}
int Renderer::GetMaxFramesInFlight() {
	return MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer& Renderer::GetFrameCommandBuffer() {
	return commandBuffers[CurrentFrame];
}

VkDescriptorSetLayout& Renderer::GetChunksSetLayout() {
	return ChunkSetLayout;
}
VkPipelineLayout& Renderer::GetChunksPipelineLayout() {
	return ChunksPipelineLayout;
}
VmaAllocator& Renderer::GetAllocator() {
	return allocator;
}
std::mutex& Renderer::GetFrameDeletionMTX() {
	return deletionQueueMTX[CurrentFrame];
}

void Renderer::SetHandles(VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    VkPhysicalDevice physicalDevice, 
    VkDevice device, VkQueue graphicsQueue, 
    VkQueue presentQueue, VkSurfaceKHR surface, 
    uint32_t graphicsFamilyIndex, 
    uint32_t presentFamilyIndex, 
    int MAX_FRAMES_IN_FLIGHT) {

	this->instance = instance;
	this->debugMessenger = debugMessenger;
	this->physicalDevice = physicalDevice;
	this->device = device;
	this->graphicsQueue = graphicsQueue;
	this->presentQueue = presentQueue;
	this->surface = surface;
	this->graphicsFamilyIndex = graphicsFamilyIndex;
	this->presentFamilyIndex = presentFamilyIndex;
    this->MAX_FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;
}
void Renderer::BindVoxelDescriptor() {
	vkCmdBindDescriptorSets(commandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ChunksPipelineLayout, 0, 1, &ChunkSets[CurrentFrame], 0, nullptr);
}
void Renderer::SetViewProj(glm::mat4 view, glm::mat4 proj) {
	MatricesBufferStruct MBO = {proj, view};
	memcpy(ChunkBuffer.BuffersMapped[CurrentFrame], &MBO, sizeof(MBO));
}
void Renderer::SetTrans(glm::mat4 trans) {
	vkCmdPushConstants(GetFrameCommandBuffer(), GetChunksPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &trans);
}



void Renderer::createCommandPool() {
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = graphicsFamilyIndex;

	if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create command pool for graphics");
	}
}
void Renderer::createCommandBuffers() {
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create one or more of the main command buffers");
	}
}
void Renderer::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolSize poolSize2{};
	poolSize2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize2.descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolSize poolSize3{};
	poolSize3.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize3.descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT;

	uint32_t count = 3;
	VkDescriptorPoolSize poolSizes[] = { poolSize, poolSize2, poolSize3 };

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = count;
	createInfo.pPoolSizes = poolSizes;
	createInfo.maxSets = 1 * MAX_FRAMES_IN_FLIGHT;

	if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create descriptor pool");
	}
}
void Renderer::createTextureSampler() {
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_NEAREST;
	createInfo.minFilter = VK_FILTER_NEAREST;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	createInfo.anisotropyEnable = VK_FALSE;
	createInfo.maxAnisotropy = 1.0f;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(device, &createInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create sampler");
	}
}
void Renderer::createAllocator() {
	VmaAllocatorCreateInfo createInfo{};
	createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
	createInfo.device = device;
	createInfo.instance = instance;
	createInfo.physicalDevice = physicalDevice;

	if(vmaCreateAllocator(&createInfo, &allocator) != VK_SUCCESS) {
		throw std::runtime_error("failed to create the Vulkan allocator");
	}
	
}
void Renderer::CreateChunkSets() {
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

    uint32_t bindingCount = 2;
    VkDescriptorSetLayoutBinding bindings[] = {MatricesBinding, TextureAtlasBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = bindings;
    layoutInfo.bindingCount = bindingCount;

    if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &ChunkSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create chunks' descriptor set layout");
    }


	//create the pipeline layout
	VkPushConstantRange modelTransformRange{};
	modelTransformRange.offset = 0;
	modelTransformRange.size = sizeof(glm::mat4);
	modelTransformRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &ChunkSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &modelTransformRange;

	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &ChunksPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create chunks pipeline layout");
	}



    //create the descriptor set/s
    std::vector<VkDescriptorSetLayout> setLayouts(MAX_FRAMES_IN_FLIGHT, ChunkSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pSetLayouts = setLayouts.data();
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    
    ChunkSets.resize(MAX_FRAMES_IN_FLIGHT);
    if(vkAllocateDescriptorSets(device, &allocInfo, ChunkSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate the descriptor sets of the Chunks");
    }


    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo MatricesBufferInfo{};
        MatricesBufferInfo.buffer = ChunkBuffer.Buffers[i];
        MatricesBufferInfo.offset = 0;
        MatricesBufferInfo.range = MatricesB_SIZE;

        VkDescriptorImageInfo AtlasInfo{};
        AtlasInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        AtlasInfo.imageView = App::Get()->m_TerrainAtlas.Get();
        AtlasInfo.sampler = sampler;



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
 
        uint32_t writeCount = 2;
        VkWriteDescriptorSet descriptorWrites[] = {MatricesWrite, AtlasWrite};
        vkUpdateDescriptorSets(device, writeCount, descriptorWrites, 0, nullptr);
	}
}


void UniformBuffer::Create(VkDeviceSize bufferSize, bool IsUniform) {
	Renderer& renderer = App::Get()->m_Renderer;
	Buffers.resize(renderer.GetMaxFramesInFlight());
	BuffersMemory.resize(renderer.GetMaxFramesInFlight());
	BuffersMapped.resize(renderer.GetMaxFramesInFlight());

	VkBufferUsageFlagBits usage = IsUniform ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	for (int i = 0; i < renderer.GetMaxFramesInFlight(); i++) {
		VKUtils::createBuffer(bufferSize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffers[i], BuffersMemory[i]);
		vkMapMemory(renderer.GetDevice(), BuffersMemory[i], 0, bufferSize, 0, &BuffersMapped[i]);
	}
}
void UniformBuffer::Delete() {
	Renderer& renderer = App::Get()->m_Renderer;
	for (int i = 0; i < renderer.GetMaxFramesInFlight(); i++) {
		vkUnmapMemory(renderer.GetDevice(), BuffersMemory[i]);
		vkDestroyBuffer(renderer.GetDevice(), Buffers[i], nullptr);
		vkFreeMemory(renderer.GetDevice(), BuffersMemory[i], nullptr);
	}
}