#pragma once
#include "Texture.h"
#include <vector>
#include <optional>

struct QueueFamilyIndicies {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct GLFWwindow; //forward declaring GLFWwindow
class Context {
public:
	void InitGPU(GLFWwindow* window);
	void TerminateGPU();

	//getters
	VkInstance GetInstance() const;
	VkDebugUtilsMessengerEXT GetDebugMessenger() const;
	VkSurfaceKHR GetSurface() const;
	VkPhysicalDevice GetPhysicalDevice() const;
	VkDevice GetDevice() const;
	VkQueue GetGraphicsQueue() const;
	VkQueue GetPresentQueue() const;
	uint32_t GetGraphicsFamily() const;
	uint32_t GetPresentFamily() const;
	VkCommandPool GetCommandPool() const;
	VkCommandBuffer* GetCommandBuffers();
	VmaAllocator GetAllocator() const;
	VkDescriptorSetLayout GetSingleTexLayout() const;
	VkPipelineLayout GetSingleTexPPLayout() const;
	Texture& GetDummyTexture();

	//public helpers
	QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	const int MAX_FRAMES_IN_FLIGHT = 3;
	int currentFrame = 0;
private:
	//initiating Vulkan
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VmaAllocator allocator;

	//layouts that are going to be repeated for both the editor and game, such as single texture layouts for in game UI or PBR texture layouts for meshes
	VkDescriptorSetLayout singleTexLayout;
	VkPipelineLayout singleTexPipelineLayout;

	VkDescriptorSetLayout pbrTexLayout;
	VkPipelineLayout pbrTexPipelineLayout;

	//default texture
	Texture dummyTexture;



	//vulkan creation functions
	void createInstance();
	void setupDebugMessenger();
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();
	void createCommandBuffers();
	void createAllocator();

	//vulkan secondary creation functions
	void createSingleTexLayouts();



	//Vulkan helper functions
	bool checkValidationLayersSupport();
	std::vector<const char*> getRequiredExtensions();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtentionSupport(VkPhysicalDevice device);
};

extern Context* GContext;