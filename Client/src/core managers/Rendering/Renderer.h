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

	void StartRender();
	void EndRender();

    //vulkan renderer getters
	VkInstance GetInstance();
	VkPhysicalDevice GetPhysicalDevice();
	VkDevice GetDevice();
	VkSurfaceKHR GetSurface();
	VkQueue GetGraphicsQueue();
	VkQueue GetPresentQueue();
	uint32_t GetGraphicsFamilyIndex();
	VkCommandPool GetCommandPool();
	VkCommandBuffer GetFrameCommandBuffer();
	int GetMaxFramesInFlight();

	VkRenderPass GetOffscreenRenderPass();
	VkDescriptorSetLayout GetChunksSetLayout();
	VkPipelineLayout GetChunksPipelineLayout();
	VmaAllocator GetAllocator();
	VkPipelineLayout GetFullscreenPipelineLayout();
	VkImage GetColorBuffer();

	std::mutex& GetFrameDeletionMTX();

    //public helpers
    void SetHandles(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkSurfaceKHR surface, uint32_t graphicsFamilyIndex, uint32_t presentFamilyIndex, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, int MAX_FRAMES_IN_FLIGHT, int* currentFrame);
	void BindVoxelDescriptor();
	void BindFullscreenQuad();
	void UpdateFullscreenQuad();
	void SetViewProj(glm::mat4 view, glm::mat4 proj);
	void SetTrans(glm::mat4 trans);
	void QueueBufferDeletion(BufferAllocation buffer);
	void FlushBufferDeletionQueue();


    int* CurrentFrame = nullptr;
private:
    //Vulkan setup handles (already done externally, will just be passing references)
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	uint32_t graphicsFamilyIndex;
	uint32_t presentFamilyIndex;
	VkCommandPool commandPool;
	VkCommandBuffer* commandBuffers;
    int MAX_FRAMES_IN_FLIGHT;

    //initiating rendering
	VkRenderPass offscreenRenderPass;
	std::vector<VkFramebuffer> offscreenFramebuffer;
	VkDescriptorPool descriptorPool;
	VkSampler sampler; //this sampler is for nearest filtering
	VmaAllocator allocator;

	//color buffer
	std::vector<VkImage> colorBuffer;
	std::vector<VkImageView> colorBufferView;
	std::vector<VmaAllocation> colorBufferAlloc;

	//depth buffer
	std::vector<VkImage> depthBuffer;
	std::vector<VkImageView> depthBufferView;
	std::vector<VmaAllocation> depthBufferAlloc;

    //descriptor sets/layouts and uniform buffers
    UniformBuffer ChunkBuffer;
    std::vector<VkDescriptorSet> ChunkSets;
    VkDescriptorSetLayout ChunkSetLayout;
	VkPipelineLayout ChunksPipelineLayout;

	VkDescriptorSetLayout fullscreenSetLayout;
	std::vector<VkDescriptorSet> fullscreenSets;
	VkPipelineLayout fullscreenPipelineLayout;

	//deletion stuff
	std::mutex deletionQueueMTX[3];
	std::vector<BufferAllocation> bufferDeletionQueue[3]; //3 because the max frames in flight is hardcoded to 3

    //creation functions
	void createOffscreenPass();
	void createOffscreenFramebuffer();
	void createColorBuffer();
	void createDepthBuffer();
    void createDescriptorPool();
    void createTextureSampler();
	void createAllocator();
    void CreateChunkSets();
	void createFullscreenSets();
};


struct MatricesBufferStruct {
    glm::mat4 proj;
    glm::mat4 view;
};