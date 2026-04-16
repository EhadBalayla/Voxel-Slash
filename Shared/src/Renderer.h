#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>

#include "vk_mem_alloc.h"

class Renderer {
public:
    void Init();
    void Terminate();

	void StartGPass();
	void EndGPass();

	void PerformLightPass();

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
	VkRenderPass GetLightingRenderPass();
	VmaAllocator GetAllocator();
	VkImage* GetColorBuffers();
	VkImage GetColorBuffer();
	VkImageView* GetColorBufferViews();
	VkImageView GetColorBufferView();
	VkImage GetLightBuffer();
	VkImageView GetLightBufferView();
	VkPipelineLayout GetGBufferPPLayout();
	VkPipelineLayout Get3DPipelineLayout();

	VkPipeline* LightShader; //temporary, later will rework into the
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
	VkDescriptorPool descriptorPool; //for all of the basic rendering stuff
	VkRenderPass offscreenRenderPass;
	VkRenderPass lightingRenderPass;
	std::vector<VkFramebuffer> offscreenFramebuffer;
	std::vector<VkFramebuffer> lightingFramebuffer;
	VkSampler sampler; //this sampler is for nearest filtering

	//gbuffer
	//color buffer
	std::vector<VkImage> colorBuffer;
	std::vector<VkImageView> colorBufferView;
	std::vector<VmaAllocation> colorBufferAlloc;

	//metalness roughness specular buffer
	std::vector<VkImage> mrsBuffer;
	std::vector<VkImageView> mrsBufferView;
	std::vector<VmaAllocation> mrsBufferAlloc;

	//normal buffer
	std::vector<VkImage> normalBuffer;
	std::vector<VkImageView> normalBufferView;
	std::vector<VmaAllocation> normalBufferAlloc;

	//position buffer
	std::vector<VkImage> positionBuffer;
	std::vector<VkImageView> positionBufferView;
	std::vector<VmaAllocation> positionBufferAlloc;

	//depth buffer
	std::vector<VkImage> depthBuffer;
	std::vector<VkImageView> depthBufferView;
	std::vector<VmaAllocation> depthBufferAlloc;

	//descriptor sets
	VkPipelineLayout GBufferPPLayout;
	VkDescriptorSetLayout GBufferSetLayout;
	std::vector<VkDescriptorSet> GBufferSets;
	//end of gbuffer


	//lighting buffer
	std::vector<VkImage> lightBuffer;
	std::vector<VkImageView> lightBufferView;
	std::vector<VmaAllocation> lightBufferAlloc;
	//end of lighting buffer


	VkPipelineLayout Pipe3DLayout;

    //creation functions
	void createDescriptorPool();
	void createOffscreenPass();
	void createLightingPass();
	void createOffscreenFramebuffer();
	void createLightingFramebuffer();
	void createColorBuffer();
	void createMRSBuffer();
	void createNormalBuffer();
	void createPositionBuffer();
	void createDepthBuffer();
	void createGBufferDescriptors();
	void createLightBuffer();
    void createTextureSampler();
	void create3DLayout();
};

extern Renderer* GRenderer;