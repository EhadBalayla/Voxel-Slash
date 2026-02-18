#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../app.h"
#include "VulkanUtilities.h"

#include <stdexcept>

void Texture::LoadTexture(const char* path) {
    unsigned char* data = stbi_load(path, &m_Width, &m_Height, &m_Channels, 4);
    if (!data) {
        throw std::runtime_error("Failed to load texture");
    }

    VkDeviceSize imageSize = m_Width * m_Height * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VKUtils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* vData;
    vkMapMemory(GApp->m_Renderer.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &vData);
    memcpy(vData, data, imageSize);
    vkUnmapMemory(GApp->m_Renderer.GetDevice(), stagingBufferMemory);


    VKUtils::createImage(m_Width, m_Height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
    VKUtils::transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VKUtils::copyBufferToImage(stagingBuffer, image, m_Width, m_Height);
    VKUtils::transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(GApp->m_Renderer.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(GApp->m_Renderer.GetDevice(), stagingBufferMemory, nullptr);


    imageView = VKUtils::createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    stbi_image_free(data);
}

void Texture::UnloadTexture() {
    vkDestroyImageView(GApp->m_Renderer.GetDevice(), imageView, nullptr);
    vkFreeMemory(GApp->m_Renderer.GetDevice(), imageMemory, nullptr);
    vkDestroyImage(GApp->m_Renderer.GetDevice(), image, nullptr);
}


VkImageView& Texture::Get() {
    return imageView;
}