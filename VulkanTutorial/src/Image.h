#pragma once

#include "vulkan/vulkan.h"
#include "Device.h"
#include "CommandPool.h"

class Image {
public:
	Image(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);	// Constructor
	~Image();	// Destructor

	// FUNCTIONS
	void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);			// Copy buffer of data to image
	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

	// GETTERS
	VkImage GetImage() { return m_Image; }
	VkDeviceMemory GetImageMemory() { return m_ImageMemory; }

private:
	// VARIABLES
	VkImage m_Image;				// Vulkan image
	VkDeviceMemory m_ImageMemory;	// Vulkan image memory
	Device* m_Device;				// Device object
	CommandPool* m_CommandPool;		// Command pool object
	VkFormat m_Format;			// Image format

	// FUNCTIONS
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);		// Find required memory type
};