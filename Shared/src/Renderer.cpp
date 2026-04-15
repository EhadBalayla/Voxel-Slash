#include "Renderer.h"

#include <stdexcept>
#include <cstring>

#include "Window.h"

Renderer* GRenderer = nullptr;

void Renderer::Init() {
	GRenderer = this;
	
	this->instance = GContext->GetInstance();
	this->physicalDevice = GContext->GetPhysicalDevice();
	this->device = GContext->GetDevice();
	this->graphicsQueue = GContext->GetGraphicsQueue();
	this->presentQueue = GContext->GetPresentQueue();
	this->surface = GContext->GetSurface();
	this->graphicsFamilyIndex = GContext->GetGraphicsFamily();
	this->presentFamilyIndex = GContext->GetPresentFamily();
	this->commandPool = GContext->GetCommandPool();
	this->commandBuffers = GContext->GetCommandBuffers();
	this->allocator = GContext->GetAllocator();
	this->MAX_FRAMES_IN_FLIGHT = GContext->MAX_FRAMES_IN_FLIGHT;
	this->CurrentFrame = &GContext->currentFrame;

	createOffscreenPass();
	createColorBuffer();
	createMRSBuffer();
	createNormalBuffer();
	createPositionBuffer();
	createDepthBuffer();
	createOffscreenFramebuffer();

	createTextureSampler();

	create3DLayout();
}
void Renderer::Terminate() {
	vkDestroyPipelineLayout(device, Pipe3DLayout, nullptr);

	vkDestroySampler(device, sampler, nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFramebuffer(device, offscreenFramebuffer[i], nullptr);

		vkDestroyImageView(device, colorBufferView[i], nullptr);
		vkDestroyImageView(device, depthBufferView[i], nullptr);

		vmaDestroyImage(allocator, colorBuffer[i], colorBufferAlloc[i]);
		vmaDestroyImage(allocator, depthBuffer[i], depthBufferAlloc[i]);
	}
	vkDestroyRenderPass(device, offscreenRenderPass, nullptr);
}

void Renderer::StartGPass() {
	uint32_t clearValueCount = 5;
	VkClearValue clearValues[] = { {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}};

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = offscreenRenderPass;
	beginInfo.framebuffer = offscreenFramebuffer[*CurrentFrame];
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent = { static_cast<uint32_t>(GWindow->GetWindowWidth()), static_cast<uint32_t>(GWindow->GetWindowHeight()) };
	beginInfo.clearValueCount = clearValueCount;
	beginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(commandBuffers[*CurrentFrame], &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(GWindow->GetWindowWidth());
	viewport.height = static_cast<float>(GWindow->GetWindowHeight());
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffers[*CurrentFrame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { (uint32_t)GWindow->GetWindowWidth(), (uint32_t)GWindow->GetWindowHeight() };
	vkCmdSetScissor(commandBuffers[*CurrentFrame], 0, 1, &scissor);
}
void Renderer::EndGPass() {
	vkCmdEndRenderPass(commandBuffers[*CurrentFrame]);
}

void Renderer::RecreateOffscreenBuffer() {
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFramebuffer(device, offscreenFramebuffer[i], nullptr);

		vkDestroyImageView(device, colorBufferView[i], nullptr);
		vkDestroyImageView(device, mrsBufferView[i], nullptr);
		vkDestroyImageView(device, normalBufferView[i], nullptr);
		vkDestroyImageView(device, positionBufferView[i], nullptr);
		vkDestroyImageView(device, depthBufferView[i], nullptr);

		vmaDestroyImage(allocator, colorBuffer[i], colorBufferAlloc[i]);
		vmaDestroyImage(allocator, mrsBuffer[i], mrsBufferAlloc[i]);
		vmaDestroyImage(allocator, normalBuffer[i], normalBufferAlloc[i]);
		vmaDestroyImage(allocator, positionBuffer[i], positionBufferAlloc[i]);
		vmaDestroyImage(allocator, depthBuffer[i], depthBufferAlloc[i]);
	}

	createColorBuffer();
	createMRSBuffer();
	createNormalBuffer();
	createPositionBuffer();
	createDepthBuffer();
	createOffscreenFramebuffer();
}

VkInstance Renderer::GetInstance() {
	return instance;
}
VkPhysicalDevice Renderer::GetPhysicalDevice() {
	return physicalDevice;
}
VkDevice Renderer::GetDevice() {
	return device;
}
VkSurfaceKHR Renderer::GetSurface() {
	return surface;
}
VkQueue Renderer::GetGraphicsQueue() {
	return graphicsQueue;
}
VkQueue Renderer::GetPresentQueue() {
	return presentQueue;
}
uint32_t Renderer::GetGraphicsFamilyIndex() {
	return graphicsFamilyIndex;
}
VkCommandPool Renderer::GetCommandPool() {
	return commandPool;
}
VkCommandBuffer Renderer::GetFrameCommandBuffer() {
	return commandBuffers[*CurrentFrame];
}
int Renderer::GetMaxFramesInFlight() {
	return MAX_FRAMES_IN_FLIGHT;
}


VkSampler Renderer::GetSampler() {
	return sampler;
}
VkRenderPass Renderer::GetOffscreenRenderPass() {
	return offscreenRenderPass;
}
VmaAllocator Renderer::GetAllocator() {
	return allocator;
}
VkImage* Renderer::GetColorBuffers() {
	return colorBuffer.data();
}
VkImage Renderer::GetColorBuffer() {
	return colorBuffer[*CurrentFrame];
}
VkImageView* Renderer::GetColorBufferViews() {
	return colorBufferView.data();
}
VkImageView Renderer::GetColorBufferView() {
	return colorBufferView[*CurrentFrame];
}
VkPipelineLayout Renderer::Get3DPipelineLayout() {
	return Pipe3DLayout;
}

void Renderer::createColorBuffer() {
	colorBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	colorBufferView.resize(MAX_FRAMES_IN_FLIGHT);
	colorBufferAlloc.resize(MAX_FRAMES_IN_FLIGHT);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.arrayLayers = 1;
	imageInfo.mipLevels = 1;
	imageInfo.extent.width = GWindow->GetWindowWidth();
	imageInfo.extent.height = GWindow->GetWindowHeight();
	imageInfo.extent.depth = 1;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &colorBuffer[i], &colorBufferAlloc[i], nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer image");
		}

		imageViewInfo.image = colorBuffer[i];

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &colorBufferView[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer view");
		}
	}
}
void Renderer::createDepthBuffer() {
	depthBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	depthBufferView.resize(MAX_FRAMES_IN_FLIGHT);
	depthBufferAlloc.resize(MAX_FRAMES_IN_FLIGHT);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = VK_FORMAT_D32_SFLOAT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.arrayLayers = 1;
	imageInfo.mipLevels = 1;
	imageInfo.extent.width = GWindow->GetWindowWidth();
	imageInfo.extent.height = GWindow->GetWindowHeight();
	imageInfo.extent.depth = 1;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_D32_SFLOAT;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthBuffer[i], &depthBufferAlloc[i], nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer image");
		}

		imageViewInfo.image = depthBuffer[i];

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &depthBufferView[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer view");
		}
	}
}
void Renderer::createMRSBuffer() {
	mrsBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	mrsBufferView.resize(MAX_FRAMES_IN_FLIGHT);
	mrsBufferAlloc.resize(MAX_FRAMES_IN_FLIGHT);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.arrayLayers = 1;
	imageInfo.mipLevels = 1;
	imageInfo.extent.width = GWindow->GetWindowWidth();
	imageInfo.extent.height = GWindow->GetWindowHeight();
	imageInfo.extent.depth = 1;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &mrsBuffer[i], &mrsBufferAlloc[i], nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer image");
		}

		imageViewInfo.image = mrsBuffer[i];

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &mrsBufferView[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer view");
		}
	}
}
void Renderer::createNormalBuffer() {
	normalBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	normalBufferView.resize(MAX_FRAMES_IN_FLIGHT);
	normalBufferAlloc.resize(MAX_FRAMES_IN_FLIGHT);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.arrayLayers = 1;
	imageInfo.mipLevels = 1;
	imageInfo.extent.width = GWindow->GetWindowWidth();
	imageInfo.extent.height = GWindow->GetWindowHeight();
	imageInfo.extent.depth = 1;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &normalBuffer[i], &normalBufferAlloc[i] , nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer image");
		}

		imageViewInfo.image = normalBuffer[i];

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &normalBufferView[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer view");
		}
	}
}
void Renderer::createPositionBuffer() {
	positionBuffer.resize(MAX_FRAMES_IN_FLIGHT);
	positionBufferView.resize(MAX_FRAMES_IN_FLIGHT);
	positionBufferAlloc.resize(MAX_FRAMES_IN_FLIGHT);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.arrayLayers = 1;
	imageInfo.mipLevels = 1;
	imageInfo.extent.width = GWindow->GetWindowWidth();
	imageInfo.extent.height = GWindow->GetWindowHeight();
	imageInfo.extent.depth = 1;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &positionBuffer[i], &positionBufferAlloc[i], nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer image");
		}

		imageViewInfo.image = positionBuffer[i];

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &positionBufferView[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create color buffer view");
		}
	}
}
void Renderer::createOffscreenPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription mrsAttachment{};
	mrsAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
	mrsAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	mrsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	mrsAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	mrsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	mrsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	mrsAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	mrsAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription normalAttachment{};
	normalAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	normalAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription positionAttachment{};
	positionAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	positionAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	uint32_t attachmentsCount = 5;
	VkAttachmentDescription attachments[] = { colorAttachment, mrsAttachment, normalAttachment, positionAttachment, depthAttachment };

	VkAttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference mrsRef{};
	mrsRef.attachment = 1;
	mrsRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference normalRef{};
	normalRef.attachment = 2;
	normalRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference posRef{};
	posRef.attachment = 3;
	posRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 4;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	uint32_t colorAttachCount = 4;
	VkAttachmentReference colorAttachRefs[] = { colorRef, mrsRef, normalRef, posRef };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = colorAttachCount;
	subpass.pColorAttachments = colorAttachRefs;
	subpass.pDepthStencilAttachment = &depthRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = attachmentsCount;
	createInfo.pAttachments = attachments;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &createInfo, nullptr, &offscreenRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create offscreen render pass");
	}
}
void Renderer::createOffscreenFramebuffer() {
	offscreenFramebuffer.resize(MAX_FRAMES_IN_FLIGHT);

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = offscreenRenderPass;
	createInfo.width = GWindow->GetWindowWidth();
	createInfo.height = GWindow->GetWindowHeight();
	createInfo.layers = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		uint32_t attachmentCount = 5;
		VkImageView attachments[] = { colorBufferView[i], mrsBufferView[i], normalBufferView[i], positionBufferView[i], depthBufferView[i] };

		createInfo.attachmentCount = attachmentCount;
		createInfo.pAttachments = attachments;

		if (vkCreateFramebuffer(device, &createInfo, nullptr, &offscreenFramebuffer[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create framebuffer");
		}
	}
}
void Renderer::createTextureSampler() {
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_NEAREST;
	createInfo.minFilter = VK_FILTER_NEAREST;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

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
void Renderer::create3DLayout() {
	//create the pipeline layout
	VkPushConstantRange modelTransformRange{};
	modelTransformRange.offset = 0;
	modelTransformRange.size = sizeof(glm::mat4);
	modelTransformRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &modelTransformRange;

	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &Pipe3DLayout) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create chunks pipeline layout");
	}
}