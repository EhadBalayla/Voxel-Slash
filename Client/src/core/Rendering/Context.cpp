#include "Context.h"

#include <iostream>
#include <set>
#include <limits>
#include <algorithm>
#include <cstring>

#undef max

#include <GLFW/glfw3.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}



const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


const std::vector<const char*> deviceExtentions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void Context::InitGPU(GLFWwindow* window) {
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
}
void Context::TerminateGPU() {
	vkDestroyDevice(device, nullptr); //ending the device
	vkDestroySurfaceKHR(instance, surface, nullptr); //ending the surface
	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr); //ending the VkDebugMessengerUtilsEXT
	vkDestroyInstance(instance, nullptr); //ending the VkInstance
}

void Context::createInstance() {
	if (enableValidationLayers && !checkValidationLayersSupport()) {
		throw std::runtime_error("validation layers requested but not available");
	}




	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "The 2000 Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "The 2000 Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto Extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = (uint32_t)Extensions.size();
	createInfo.ppEnabledExtensionNames = Extensions.data();


	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance");
	}
}
void Context::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to create debug messenger");
	}
}
void Context::createSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("could not create window surface!!!!");
	}
}
void Context::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


	for (const auto& n : devices) {
		if (isDeviceSuitable(n)) {
			physicalDevice = n;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}
void Context::createLogicalDevice() {
	QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indicies.graphicsFamily.value(), indicies.presentFamily.value() };

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}


	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.multiDrawIndirect = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;


	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	//gets the extentions for the logical device
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtentions.size());
	createInfo.ppEnabledExtensionNames = deviceExtentions.data();

	//if validation layers enabled, get them bitches for the logical device
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device");
	}

	vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphicsQueue); //gets the queue for the graphics family index
	vkGetDeviceQueue(device, indicies.presentFamily.value(), 0, &presentQueue); //gets the queue for the presentation family index

	graphicsFamily = indicies.graphicsFamily.value();
	presentFamily = indicies.presentFamily.value();
}



VkInstance& Context::GetInstance() {
	return instance;
}
VkDebugUtilsMessengerEXT& Context::GetDebugMessenger() {
	return debugMessenger;
}
VkPhysicalDevice& Context::GetPhysicalDevice() {
	return physicalDevice;
}
VkDevice& Context::GetDevice() {
	return device;
}
VkSurfaceKHR& Context::GetSurface() {
	return surface;
}
VkQueue& Context::GetGraphicsQueue() {
	return graphicsQueue;
}
VkQueue& Context::GetPresentQueue() {
	return presentQueue;
}
uint32_t Context::GetGraphicsFamily() {
	return graphicsFamily;
}
uint32_t Context::GetPresentFamily() {
	return presentFamily;
}



QueueFamilyIndicies Context::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndicies indicies;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indicies.graphicsFamily = i;
		}



		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
			indicies.presentFamily = i;



		if (indicies.IsComplete()) {
			break;
		}

		i++;
	}

	return indicies;
}
SwapChainSupportDetails Context::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);


	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}


	uint32_t presentModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModes, nullptr);

	if (presentModes != 0) {
		details.presentModes.resize(presentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModes, details.presentModes.data());
	}

	return details;
}
bool Context::checkValidationLayersSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}
std::vector<const char*> Context::getRequiredExtensions() {
	uint32_t glfwExtensionsCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	std::vector<const char*> Extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	if (enableValidationLayers) {
		Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return Extensions;
}
VkResult Context::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void Context::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
void Context::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}
bool Context::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndicies indicies = findQueueFamilies(device);

	bool extentionsSupported = checkDeviceExtentionSupport(device);
	bool SwapchainAdequate = false;
	if (extentionsSupported) {
		SwapChainSupportDetails details = querySwapChainSupport(device);
		SwapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
	}

	return indicies.IsComplete() && extentionsSupported && SwapchainAdequate;
}
bool Context::checkDeviceExtentionSupport(VkPhysicalDevice device) {
	uint32_t extentionsCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extentionsCount, nullptr);

	std::vector<VkExtensionProperties> availableExtentions(extentionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extentionsCount, availableExtentions.data());

	std::set<std::string> requiredExtentions(deviceExtentions.begin(), deviceExtentions.end());
	for (const auto& extention : availableExtentions) {
		requiredExtentions.erase(extention.extensionName);
	}

	return requiredExtentions.empty();
}