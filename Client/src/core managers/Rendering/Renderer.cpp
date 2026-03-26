#include "Renderer.h"

#include <stdexcept>
#include <cstring>

#include "../app.h"

void Renderer::Init() {
	createOffscreenPass();
	createColorBuffer();
	createDepthBuffer();
	createOffscreenFramebuffer();

    createDescriptorPool();
	createTextureSampler();
}
void Renderer::StartDescriptors() {
    CreateChunkSets();
}
void Renderer::EndDescriptors() {
	vkDestroyPipelineLayout(device, ChunksPipelineLayout, nullptr);

	vkDestroyDescriptorSetLayout(device, ChunkSetLayout, nullptr);

	ChunkBuffer.Delete();
}
void Renderer::Terminate() {
	vkDestroySampler(device, sampler, nullptr);

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFramebuffer(device, offscreenFramebuffer[i], nullptr);

		vkDestroyImageView(device, colorBufferView[i], nullptr);
		vkDestroyImageView(device, depthBufferView[i], nullptr);

		vmaDestroyImage(allocator, colorBuffer[i], colorBufferAlloc[i]);
		vmaDestroyImage(allocator, depthBuffer[i], depthBufferAlloc[i]);
	}
	vkDestroyRenderPass(device, offscreenRenderPass, nullptr);
}

void Renderer::StartRender() {
	uint32_t clearValueCount = 2;
	VkClearValue clearValues[] = { {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} };

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = offscreenRenderPass;
	beginInfo.framebuffer = offscreenFramebuffer[*CurrentFrame];
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent = { static_cast<uint32_t>(GApp->Width), static_cast<uint32_t>(GApp->Height) };
	beginInfo.clearValueCount = clearValueCount;
	beginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(commandBuffers[*CurrentFrame], &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(GApp->Width);
	viewport.height = static_cast<float>(GApp->Height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffers[*CurrentFrame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { (uint32_t)GApp->Width, (uint32_t)GApp->Height };
	vkCmdSetScissor(commandBuffers[*CurrentFrame], 0, 1, &scissor);
}
void Renderer::EndRender() {
	vkCmdEndRenderPass(commandBuffers[*CurrentFrame]);
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


VkSampler Renderer::GetSampler() const {
	return sampler;
}
VkRenderPass Renderer::GetOffscreenRenderPass() const {
	return offscreenRenderPass;
}
VkDescriptorSetLayout Renderer::GetChunksSetLayout() const {
	return ChunkSetLayout;
}
VkDescriptorSetLayout Renderer::GetChunkMeshLayout() const {
	return ChunkMeshLayout;
}
VkDescriptorSet Renderer::GetChunksSet(int idx) const {
	return ChunkSets[idx];
}
VkPipelineLayout Renderer::GetChunksPipelineLayout() const {
	return ChunksPipelineLayout;
}
VmaAllocator Renderer::GetAllocator() const {
	return allocator;
}
std::mutex& Renderer::GetFrameDeletionMTX() {
	return deletionQueueMTX[*CurrentFrame];
}
VkImage Renderer::GetColorBuffer() {
	return colorBuffer[*CurrentFrame];
}
VkImageView Renderer::GetColorBufferView() {
	return colorBufferView[*CurrentFrame];
}

void Renderer::SetHandles(
	VkInstance instance, 
	VkPhysicalDevice physicalDevice, 
	VkDevice device, 
	VkQueue graphicsQueue, 
	VkQueue presentQueue, 
	VkSurfaceKHR surface, 
	uint32_t graphicsFamilyIndex, 
	uint32_t presentFamilyIndex, 
	VkCommandPool commandPool, 
	VkCommandBuffer* commandBuffers,
	VmaAllocator allocator,
	int MAX_FRAMES_IN_FLIGHT, 
	int* currentFrame) {

	this->instance = instance;
	this->physicalDevice = physicalDevice;
	this->device = device;
	this->graphicsQueue = graphicsQueue;
	this->presentQueue = presentQueue;
	this->surface = surface;
	this->graphicsFamilyIndex = graphicsFamilyIndex;
	this->presentFamilyIndex = presentFamilyIndex;
	this->commandPool = commandPool;
	this->commandBuffers = commandBuffers;
	this->allocator = allocator;
    this->MAX_FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;
	this->CurrentFrame = currentFrame;
}
void Renderer::SetViewProj(glm::mat4 view, glm::mat4 proj) {
	MatricesBufferStruct MBO = {proj, view};
	memcpy(ChunkBuffer.BuffersMapped[*CurrentFrame], &MBO, sizeof(MBO));
}
void Renderer::SetTrans(glm::mat4 trans) {
	vkCmdPushConstants(commandBuffers[*CurrentFrame], ChunksPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &trans);
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
	imageInfo.extent.width = GApp->Width;
	imageInfo.extent.height = GApp->Height;
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
	imageInfo.extent.width = GApp->Width;
	imageInfo.extent.height = GApp->Height;
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

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	uint32_t attachmentsCount = 2;
	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

	VkAttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
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
	createInfo.width = GApp->Width;
	createInfo.height = GApp->Height;
	createInfo.layers = 1;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkImageView attachments[] = { colorBufferView[i], depthBufferView[i] };

		createInfo.attachmentCount = 2;
		createInfo.pAttachments = attachments;

		if (vkCreateFramebuffer(device, &createInfo, nullptr, &offscreenFramebuffer[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create framebuffer");
		}
	}
}

void Renderer::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolSize poolSize2{};
	poolSize2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize2.descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolSize poolSize3{};
	poolSize3.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize3.descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT;

	uint32_t count = 3;
	VkDescriptorPoolSize poolSizes[] = { poolSize, poolSize2, poolSize3 };

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = count;
	createInfo.pPoolSizes = poolSizes;
	createInfo.maxSets = 2 * MAX_FRAMES_IN_FLIGHT;

	if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create descriptor pool");
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
void Renderer::CreateChunkSets() {
    //allocate the uniform buffer
    VkDeviceSize MatricesB_SIZE = sizeof(MatricesBufferStruct);

    ChunkBuffer.Create(MatricesB_SIZE, true);
    
    //create the set layout
    VkDescriptorSetLayoutBinding MatricesBinding{};
    MatricesBinding.binding = 0;
    MatricesBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    MatricesBinding.descriptorCount = 1;
    MatricesBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorSetLayoutBinding TextureAtlasBinding{};
    TextureAtlasBinding.binding = 1;
    TextureAtlasBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    TextureAtlasBinding.descriptorCount = 1;
    TextureAtlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    TextureAtlasBinding.pImmutableSamplers = nullptr;

    uint32_t bindingCount = 2;
    VkDescriptorSetLayoutBinding bindings[] = {MatricesBinding, TextureAtlasBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = bindings;
    layoutInfo.bindingCount = bindingCount;

    if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &ChunkSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create chunks' descriptor set layout");
    }

	//create the set layout for mesh buffer
	VkDescriptorSetLayoutBinding meshBinding{};
	meshBinding.binding = 0;
	meshBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	meshBinding.descriptorCount = 1;
	meshBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	VkDescriptorSetLayoutCreateInfo meshLayoutInfo{};
	meshLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	meshLayoutInfo.pBindings = &meshBinding;
	meshLayoutInfo.bindingCount = 1;

	if (vkCreateDescriptorSetLayout(device, &meshLayoutInfo, nullptr, &ChunkMeshLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create chunks mesh' descriptor set layout");
	}


	//create the pipeline layout
	VkDescriptorSetLayout ppSetLayouts[] = { ChunkSetLayout, ChunkMeshLayout };
	uint32_t setLayoutCounts = 2;

	VkPushConstantRange modelTransformRange{};
	modelTransformRange.offset = 0;
	modelTransformRange.size = sizeof(glm::mat4);
	modelTransformRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayoutCounts;
	pipelineLayoutInfo.pSetLayouts = ppSetLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &modelTransformRange;

	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &ChunksPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create chunks pipeline layout");
	}



    //create the descriptor set/s
    std::vector<VkDescriptorSetLayout> setLayouts(MAX_FRAMES_IN_FLIGHT, ChunkSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pSetLayouts = setLayouts.data();
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    
    ChunkSets.resize(MAX_FRAMES_IN_FLIGHT);
    if(vkAllocateDescriptorSets(device, &allocInfo, ChunkSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate the descriptor sets of the Chunks");
    }


    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo MatricesBufferInfo{};
        MatricesBufferInfo.buffer = ChunkBuffer.Buffers[i];
        MatricesBufferInfo.offset = 0;
        MatricesBufferInfo.range = MatricesB_SIZE;

        VkDescriptorImageInfo AtlasInfo{};
        AtlasInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        AtlasInfo.imageView = GApp->m_TerrainAtlas.GetImageView();
        AtlasInfo.sampler = sampler;



        VkWriteDescriptorSet MatricesWrite{};
        MatricesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        MatricesWrite.dstSet = ChunkSets[i];
        MatricesWrite.dstBinding = 0;
        MatricesWrite.pBufferInfo = &MatricesBufferInfo;
        MatricesWrite.dstArrayElement = 0;
        MatricesWrite.descriptorCount = 1;
        MatricesWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkWriteDescriptorSet AtlasWrite{};
        AtlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        AtlasWrite.dstSet = ChunkSets[i];
        AtlasWrite.dstBinding = 1;
        AtlasWrite.pImageInfo = &AtlasInfo;
        AtlasWrite.dstArrayElement = 0;
        AtlasWrite.descriptorCount = 1;
        AtlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
 
        uint32_t writeCount = 2;
        VkWriteDescriptorSet descriptorWrites[] = {MatricesWrite, AtlasWrite};
        vkUpdateDescriptorSets(device, writeCount, descriptorWrites, 0, nullptr);
	}
}


void UniformBuffer::Create(VkDeviceSize bufferSize, bool IsUniform) {
	Renderer& renderer = GApp->m_Renderer;
	Buffers.resize(renderer.GetMaxFramesInFlight());
	BuffersAllocation.resize(renderer.GetMaxFramesInFlight());
	BuffersMapped.resize(renderer.GetMaxFramesInFlight());

	//allocating the regular mesh buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = IsUniform ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;


	for(int i = 0; i < GContext->MAX_FRAMES_IN_FLIGHT; i++) {
		if(vmaCreateBuffer(renderer.GetAllocator(), &bufferInfo, &allocInfo, &Buffers[i], &BuffersAllocation[i], nullptr) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate mesh buffer of a chunk");
		}

    	vmaMapMemory(renderer.GetAllocator(), BuffersAllocation[i], &BuffersMapped[i]);
	}
}
void UniformBuffer::Delete() {
	Renderer& renderer = GApp->m_Renderer;
	for (int i = 0; i < renderer.GetMaxFramesInFlight(); i++) {
		vmaUnmapMemory(renderer.GetAllocator(), BuffersAllocation[i]);
		vmaDestroyBuffer(renderer.GetAllocator(), Buffers[i], BuffersAllocation[i]);
	}
}