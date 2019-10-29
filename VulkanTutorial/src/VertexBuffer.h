#pragma once

#include "vulkan/vulkan.h"

class VertexBuffer {
public:
	VertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	~VertexBuffer();
	
	void CopyToBuffer(VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkDeviceSize size);		// Copy data to buffer
	void Bind();						// Bind buffer

	VkBuffer GetBuffer() { return m_Buffer; }
	VkDeviceMemory GetBufferMemory() { return m_BufferMemory; }
private:
	// VARIABLES
	VkDevice m_Device;					// Vulkan logical device
	VkPhysicalDevice m_PhysicalDevice;	// Vulkan physical device
	VkBuffer m_Buffer;					// Vulkan buffer object
	VkDeviceMemory m_BufferMemory;		// Buffer allocated memory

	// FUNCTIONS
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);		// Find appropriate memory type
};