#pragma once

#include "vulkan/vulkan.h"
#include "Device.h"
#include "CommandPool.h"

class Image {
public:
	Image(Device* device, CommandPool* commandPool, int32_t width, int32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);	// Constructor
	~Image();	// Destructor

	// FUNCTIONS
	void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);			// Copy buffer of data to image
	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void GenerateMipmaps();			// Generate mip maps for image

	// GETTERS
	VkImage GetImage() { return m_Image; }
	VkDeviceMemory GetImageMemory() { return m_ImageMemory; }
	VkImageView GetImageView() { return m_ImageView; }
	uint32_t GetMipLevels() { return m_MipLevels; }
private:
	// VARIABLES
	VkImage m_Image;				// Vulkan image
	VkDeviceMemory m_ImageMemory;	// Vulkan image memory
	VkImageView m_ImageView;		// Vulkan image view
	Device* m_Device;				// Device object
	CommandPool* m_CommandPool;		// Command pool object
	VkFormat m_Format;				// Image format
	uint32_t m_MipLevels;			// Mip levels of image
	int32_t m_Width, m_Height;		// Width and Height of image

	// FUNCTIONS
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);		// Find required memory type
};