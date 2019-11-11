#pragma once

#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "vulkan/vulkan.h"
#include "Buffer.h"
#include "Device.h"
#include "CommandPool.h"
#include "Image.h"

class Texture {
public:
	Texture(Device* device, CommandPool* commandPool, const char* path);		// Constructor
	~Texture();		// Destructor

	// Getters
	Image* GetImage() { return m_Image; }
	VkDeviceMemory GetImageMemory() { return m_Image->GetImageMemory(); }
private:
	// VARIABLES
	Device* m_Device;		// Device object
	Image*	m_Image;		// Texture image

};