#define VMA_IMPLEMENTATION
#include "Renderer.h"

#include <stdexcept>
#include <cstring>

#include "../Editor.h"

#include "VulkanUtilities.h"

void Renderer::Init() {
    createCommandPool();
    createCommandBuffers();
    createDescriptorPool();
    createTextureSampler();
	createAllocator();
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