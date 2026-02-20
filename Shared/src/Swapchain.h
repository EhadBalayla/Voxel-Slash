#pragma once
#include <vector>
#include "vulkan/vulkan.h"

struct GLFWwindow;
class Context;
class Swapchain {
public:
	void Create();
	void Delete();

	void RecreateSwapchain() {};

	uint32_t imageIndex; //will be used for the index of the swapchain image for synchronization

	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainImageExtent;

	//the syncronization handles
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderingFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass swapchainRenderPass; //just temporary wanna see if it works... but even then there is ever going to be one swapchain in the editor to begin with

	//make sure to set later
	Context* context = nullptr;
	int maxFramesInFlight = 0;
	GLFWwindow* window = nullptr;
private:
	//creation functions
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};