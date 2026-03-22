#include "Texture.h"
#include "Context.h"

#include <stdexcept>

void Texture::Create(void* pixelData, int Width, int Height) {
    VkDeviceSize size = Width * Height * 4;

    //the staging buffer
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingInfo.size = size;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo stagingAllocInfo{};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    
    if (vmaCreateBuffer(GContext->GetAllocator(), &stagingInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create staging buffer for texture");
    }

    void* pData;
    vmaMapMemory(GContext->GetAllocator(), stagingAllocation, &pData);
    memcpy(pData, pixelData, Width * Height * 4);
    vmaUnmapMemory(GContext->GetAllocator(), stagingAllocation);

    //creating the image itself
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = 1;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.extent.width = Width;
    imageInfo.extent.height = Height;
    imageInfo.extent.depth = 1;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(GContext->GetAllocator(), &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create the texture of an image");
    }

    //temporary single use command buffer
    VkCommandBuffer tempBuffer;
    VkCommandBufferAllocateInfo tempBufferInfo{};
    tempBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    tempBufferInfo.commandBufferCount = 1;
    tempBufferInfo.commandPool = GContext->GetCommandPool();
    tempBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    if (vkAllocateCommandBuffers(GContext->GetDevice(), &tempBufferInfo, &tempBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate temporary command buffer for texture creation");
    }

    VkCommandBufferBeginInfo tempBufferBegin{};
    tempBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    tempBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(tempBuffer, &tempBufferBegin) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin the temporary command buffer for texture creation");
    }

    VkImageMemoryBarrier imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = image;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(tempBuffer, 
        srcStage, dstStage,
        0, 
        0, nullptr, 
        0, nullptr, 
        1, &imageBarrier);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { (uint32_t)Width, (uint32_t)Height, 1};

    vkCmdCopyBufferToImage(tempBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier imageBarrier2{};
    imageBarrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier2.image = image;
    imageBarrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier2.subresourceRange.baseMipLevel = 0;
    imageBarrier2.subresourceRange.levelCount = 1;
    imageBarrier2.subresourceRange.baseArrayLayer = 0;
    imageBarrier2.subresourceRange.layerCount = 1;
    imageBarrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkPipelineStageFlags srcStage2 = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkPipelineStageFlags dstStage2 = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
    vkCmdPipelineBarrier(tempBuffer,
        srcStage2, dstStage2,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageBarrier2);

    vkEndCommandBuffer(tempBuffer);

    VkSubmitInfo tempSubmitInfo{};
    tempSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    tempSubmitInfo.commandBufferCount = 1;
    tempSubmitInfo.pCommandBuffers = &tempBuffer;
    vkQueueSubmit(GContext->GetGraphicsQueue(), 1, &tempSubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(GContext->GetGraphicsQueue());

    vkFreeCommandBuffers(GContext->GetDevice(), GContext->GetCommandPool(), 1, &tempBuffer);

    //destroying the staging buffer
    vmaDestroyBuffer(GContext->GetAllocator(), stagingBuffer, stagingAllocation);

    //creating the image view
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.image = image;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;

    if (vkCreateImageView(GContext->GetDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view for texture");
    }
}
void Texture::Delete() {

}


VkImageView Texture::GetImageView() const {
    return imageView;
}