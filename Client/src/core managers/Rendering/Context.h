#pragma once
#include "vulkan/vulkan.h"
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
	VkInstance& GetInstance();
	VkDebugUtilsMessengerEXT& GetDebugMessenger();
	VkSurfaceKHR& GetSurface();
	VkPhysicalDevice& GetPhysicalDevice();
	VkDevice& GetDevice();
	VkQueue& GetGraphicsQueue();
	VkQueue& GetPresentQueue();
	uint32_t GetGraphicsFamily();
	uint32_t GetPresentFamily();

	//public helpers
	QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	const int MAX_FRAMES_IN_FLIGHT = 3;
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

	//vulkan creation functions
	void createInstance();
	void setupDebugMessenger();
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();



	//Vulkan helper functions
	bool checkValidationLayersSupport();
	std::vector<const char*> getRequiredExtensions();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtentionSupport(VkPhysicalDevice device);
};