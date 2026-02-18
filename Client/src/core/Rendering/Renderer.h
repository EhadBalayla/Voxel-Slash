#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>

#include "vk_mem_alloc.h"

#include <mutex>


struct UniformBuffer {
	std::vector<VkBuffer> Buffers;
	std::vector<VkDeviceMemory> BuffersMemory;
	std::vector<void*> BuffersMapped;

	void Create(VkDeviceSize bufferSize, bool IsUniform /*true if uniform buffer, false if storage buffer*/);
	void Delete();
};

struct BufferAllocation {
	VkBuffer buffer;
	VmaAllocation allocation;
	bool tbuffer;
};

class Renderer {
public:
    void Init();
    void StartDescriptors();
	void EndDescriptors();
    void Terminate();

    //vulkan renderer getters
	VkInstance& GetInstance();
	VkPhysicalDevice& GetPhysicalDevice();
	VkDevice& GetDevice();
	VkSurfaceKHR& GetSurface();
	VkCommandPool& GetCommandPool();
	VkQueue& GetGraphicsQueue();
	VkQueue& GetPresentQueue();
	uint32_t GetGraphicsFamilyIndex();
	int GetMaxFramesInFlight();

	VkCommandBuffer& GetFrameCommandBuffer(); //returns the command buffer of the current frame

	VkDescriptorSetLayout& GetChunksSetLayout();
	VkPipelineLayout& GetChunksPipelineLayout();
	VmaAllocator& GetAllocator();
	VkBuffer& GetGigabuffer();
	VmaVirtualBlock& GetGigablock();

	VkCommandPool& GetUploadPool();
	VkQueue& GetUploadQueue();
	std::mutex& GetFrameDeletionMTX();

    //public helpers
    void SetHandles(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkSurfaceKHR surface, uint32_t graphicsFamilyIndex, uint32_t presentFamilyIndex, int MAX_FRAMES_IN_FLIGHT);
	void BindVoxelDescriptor();
	void SetViewProj(glm::mat4 view, glm::mat4 proj);
	void SetTrans(glm::mat4 trans);
	void QueueBufferDeletion(BufferAllocation buffer);
	void FlushBufferDeletionQueue();


    int CurrentFrame = 0;
private:
    //Vulkan setup handles (already done externally, will just be passing references)
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	uint32_t graphicsFamilyIndex;
	uint32_t presentFamilyIndex;
    int MAX_FRAMES_IN_FLIGHT;

    //initiating rendering
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkDescriptorPool descriptorPool;
	VkSampler sampler; //this sampler is for nearest filtering
	VmaAllocator allocator;

    //descriptor sets/layouts and uniform buffers
    UniformBuffer ChunkBuffer;
    std::vector<VkDescriptorSet> ChunkSets;
    VkDescriptorSetLayout ChunkSetLayout;
	VkPipelineLayout ChunksPipelineLayout;

	//deletion stuff
	std::mutex deletionQueueMTX[3];
	std::vector<BufferAllocation> bufferDeletionQueue[3]; //3 because the max frames in flight is hardcoded to 3

    //creation functions
    void createCommandPool();
	void createCommandBuffers();
    void createDescriptorPool();
    void createTextureSampler();
	void createAllocator();
    void CreateChunkSets();
};


struct MatricesBufferStruct {
    glm::mat4 proj;
    glm::mat4 view;
};