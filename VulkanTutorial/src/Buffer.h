#pragma once

#include "vulkan/vulkan.h"

typedef enum BufferType {
	UNDEFINED_BUFFER,
	VERTEX_BUFFER,
	INDEX_BUFFER
} BufferType;

class Buffer {
public:
	Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType bufferType = UNDEFINED_BUFFER);
	~Buffer();
	
	void CopyToBuffer(VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkDeviceSize size);		// Copy data to buffer

	VkBuffer GetBuffer() { return m_Buffer; }
	VkDeviceMemory GetBufferMemory() { return m_BufferMemory; }
private:
	// VARIABLES
	BufferType m_BufferType;			// Buffer type (vertex or index)
	VkDevice m_Device;					// Vulkan logical device
	VkPhysicalDevice m_PhysicalDevice;	// Vulkan physical device
	VkBuffer m_Buffer;					// Vulkan buffer object
	VkDeviceMemory m_BufferMemory;		// Buffer allocated memory

	// FUNCTIONS
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);		// Find appropriate memory type
};