#include "Swapchain.h"
#include "Context.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <algorithm>
#include <limits>

void Swapchain::Create() {
	createSwapChain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createSyncObjects();
}
void Swapchain::Delete() {
	for(int i = 0; i < maxFramesInFlight; i++) {
		vkDestroySemaphore(context->GetDevice(), imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(context->GetDevice(), inFlightFences[i], nullptr);
	}
	for(auto semaphore : renderingFinishedSemaphores)
		vkDestroySemaphore(context->GetDevice(), semaphore, nullptr);
	for (auto Framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(context->GetDevice(), Framebuffer, nullptr);
    vkDestroyRenderPass(context->GetDevice(), swapchainRenderPass, nullptr);
	for (auto ImageView : swapChainImageViews)
		vkDestroyImageView(context->GetDevice(), ImageView, nullptr);
	vkDestroySwapchainKHR(context->GetDevice(), swapchain, nullptr);
}



void Swapchain::createSwapChain() {
	SwapChainSupportDetails details = context->querySwapChainSupport(context->GetPhysicalDevice());

	VkSurfaceFormatKHR chosenFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR chosenPresent = chooseSwapPresentMode(details.presentModes);
	VkExtent2D chosenExtent = chooseSwapExtent(details.capabilities);

	uint32_t imageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = context->GetSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = chosenFormat.format;
	createInfo.imageColorSpace = chosenFormat.colorSpace;
	createInfo.imageExtent = chosenExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndicies[] = { context->GetGraphicsFamily(), context->GetPresentFamily()};
	if (queueFamilyIndicies[0] != queueFamilyIndicies[1]) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndicies;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chosenPresent;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(context->GetDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create a swapchain");
	}

	vkGetSwapchainImagesKHR(context->GetDevice(), swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(context->GetDevice(), swapchain, &imageCount, swapchainImages.data());

	swapchainImageFormat = chosenFormat.format;
	swapchainImageExtent = chosenExtent;
}
void Swapchain::createImageViews() {
	swapChainImageViews.resize(swapchainImages.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(context->GetDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create swapchain image views");
		}
	}
}
void Swapchain::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	uint32_t attachmentCount = 1;
	VkAttachmentDescription attachments[] = { colorAttachment };


	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(context->GetDevice(), &renderPassInfo, nullptr, &swapchainRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain render pass!");
	}
}
void Swapchain::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		uint32_t attachmentCount = 1;
		VkImageView attachments[] = {swapChainImageViews[i]};

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = swapchainRenderPass;
		createInfo.attachmentCount = attachmentCount;
		createInfo.pAttachments = attachments;
		createInfo.width = swapchainImageExtent.width;
		createInfo.height = swapchainImageExtent.height;
		createInfo.layers = 1;

		if (vkCreateFramebuffer(context->GetDevice(), &createInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create framebuffer");
		}
	}
}
void Swapchain::createSyncObjects() {
	imageAvailableSemaphores.resize(maxFramesInFlight);
	inFlightFences.resize(maxFramesInFlight);
	renderingFinishedSemaphores.resize(swapchainImages.size());

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < maxFramesInFlight; i++) {
		if (vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(context->GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects of the swapchain for a frame");
		}
	}
	for(int i = 0; i < swapchainImages.size(); i++) {
		if(vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &renderingFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create second syncronization objects of the swapchain for a frame");
		}
	}
}


VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& format : availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return availableFormats[0];
}
VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& present : availablePresentModes) {
		if (present == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			return present;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}