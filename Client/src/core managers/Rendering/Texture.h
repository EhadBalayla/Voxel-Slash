#pragma once
#include <vulkan/vulkan.h>


class Texture
{
public:
	void UnloadTexture();
    void LoadTexture(const char* path);

	inline int getWidth() const { return m_Width; }
	inline int getHeight() const { return m_Height; }

	VkImageView& Get();
private:
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;

	int m_Width, m_Height, m_Channels;
};