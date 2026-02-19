#pragma once
#include "vulkan/vulkan.h"

//amount of blocks in chunk * faces per block * verticies per face * size of each vertex
constexpr size_t theoreticalMaxVerticies = 16 * 16 * 16 * 6 * 4 * sizeof(uint32_t);

//amount of blocks * faces per blocks * 6 indicies per face * size of each index
constexpr size_t theoreticalMaxIndicies = 16 * 16 * 16 * 6 * 6 * sizeof(uint32_t);

namespace VKUtils {
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	VkCommandBuffer BeginSingleUseCommandBuffer();
	void EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer);

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	bool hasStencilComponent(VkFormat format);
}