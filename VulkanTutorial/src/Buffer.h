#pragma once

#include "vulkan/vulkan.h"

typedef enum BufferType {
	BUFFER_UNDEFINED,
	BUFFER_VERTEX,
	BUFFER_INDEX,
	BUFFER_UNIFORM
} BufferType;

class Buffer {
public:
	Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType bufferType = BUFFER_UNDEFINED);
	~Buffer();
	
	void CopyToBuffer(VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkDeviceSize size);		// Copy data to buffer
	void Bind(VkCommandBuffer commandBuffer);		// Bind buffer to commandbuffer

	VkBuffer GetBuffer() { return m_Buffer; }
	VkDeviceMemory GetBufferMemory() { return m_BufferMemory; }
private:
	// VARIABLES
	BufferType m_BufferType;			// Buffer type
	VkDevice m_Device;					// Vulkan logical device
	VkPhysicalDevice m_PhysicalDevice;	// Vulkan physical device
	VkBuffer m_Buffer;					// Vulkan buffer object
	VkDeviceMemory m_BufferMemory;		// Buffer allocated memory

	// FUNCTIONS
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);		// Find appropriate memory type
};