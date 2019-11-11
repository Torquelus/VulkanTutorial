#pragma once

#include "vulkan/vulkan.h"

#include "Device.h"

class ImageView {
public:
	ImageView(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);	// Constructor
	~ImageView();	// Destructor

	// GETTERS
	VkImageView GetImageView() { return m_ImageView; }
private:
	// VARIABLES
	VkImageView m_ImageView;	// Vulkan Image view
	Device* m_Device;			// Vulkan device
};