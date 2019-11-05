#pragma once

#include "vulkan/vulkan.h"
#include "Device.h"

typedef enum BufferType {
	BUFFER_UNDEFINED,
	BUFFER_VERTEX,
	BUFFER_INDEX,
	BUFFER_UNIFORM
} BufferType;

class Buffer {
public:
	Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType bufferType = BUFFER_UNDEFINED);
	~Buffer();
	
	void CopyToBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkDeviceSize size);		// Copy data to buffer
	void Bind(VkCommandBuffer commandBuffer);		// Bind buffer to commandbuffer

	VkBuffer GetBuffer() { return m_Buffer; }
	VkDeviceMemory GetBufferMemory() { return m_BufferMemory; }
private:
	// VARIABLES
	BufferType m_BufferType;			// Buffer type
	Device* m_Device;					// Vulkan device
	VkBuffer m_Buffer;					// Vulkan buffer object
	VkDeviceMemory m_BufferMemory;		// Buffer allocated memory

	// FUNCTIONS
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);		// Find appropriate memory type
};