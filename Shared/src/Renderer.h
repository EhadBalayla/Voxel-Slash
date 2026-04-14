#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>

#include "vk_mem_alloc.h"

class Renderer {
public:
    void Init();
    void Terminate();

	void StartRender();
	void EndRender();

	void RecreateOffscreenBuffer();

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

	VkSampler GetSampler();
	VkRenderPass GetOffscreenRenderPass();
	VmaAllocator GetAllocator();
	VkImage* GetColorBuffers();
	VkImage GetColorBuffer();
	VkImageView* GetColorBufferViews();
	VkImageView GetColorBufferView();
	VkPipelineLayout Get3DPipelineLayout();

    int* CurrentFrame = nullptr;
private:
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
	VmaAllocator allocator;
    int MAX_FRAMES_IN_FLIGHT;

    //initiating rendering
	VkRenderPass offscreenRenderPass;
	std::vector<VkFramebuffer> offscreenFramebuffer;
	VkSampler sampler; //this sampler is for nearest filtering

	//color buffer
	std::vector<VkImage> colorBuffer;
	std::vector<VkImageView> colorBufferView;
	std::vector<VmaAllocation> colorBufferAlloc;

	//depth buffer
	std::vector<VkImage> depthBuffer;
	std::vector<VkImageView> depthBufferView;
	std::vector<VmaAllocation> depthBufferAlloc;

	VkPipelineLayout Pipe3DLayout;

    //creation functions
	void createOffscreenPass();
	void createOffscreenFramebuffer();
	void createColorBuffer();
	void createDepthBuffer();
    void createTextureSampler();
	void create3DLayout();
};

extern Renderer* GRenderer;