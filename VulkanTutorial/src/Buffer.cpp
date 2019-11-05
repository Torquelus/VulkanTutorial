#include "Buffer.h"

#include <stdexcept>

// Constructor
Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType bufferType)
	: m_BufferType(bufferType), m_Device(device) {

	// Buffer creation info
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create buffer
	if (vkCreateBuffer(m_Device->GetDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	// Get memory requirements
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Device->GetDevice(), m_Buffer, &memRequirements);

	// Allocation info
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	// Allocate memory
	if (vkAllocateMemory(m_Device->GetDevice(), &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	// Bind buffer to memory
	vkBindBufferMemory(m_Device->GetDevice(), m_Buffer, m_BufferMemory, 0);

}

// Destructor
Buffer::~Buffer(){
	// Destroy vertex buffer and free memory
	vkDestroyBuffer(m_Device->GetDevice(), m_Buffer, nullptr);
	vkFreeMemory(m_Device->GetDevice(), m_BufferMemory, nullptr);
}

// Copy data to buffer
void Buffer::CopyToBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkDeviceSize size){

	// Command buffer allocation info
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	// Create command buffer
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_Device->GetDevice(), &allocInfo, &commandBuffer);

	// Start recording command buffer
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Create copy buffer command
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, m_Buffer, 1, &copyRegion);

	// End command buffer
	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_Device->GetGraphicsQueue());

	// Free command buffers
	vkFreeCommandBuffers(m_Device->GetDevice(), commandPool, 1, &commandBuffer);

}

void Buffer::Bind(VkCommandBuffer commandBuffer){
	switch (m_BufferType) {
	case BUFFER_VERTEX: {
		// Bind vertex buffer
		VkBuffer buffers[] = { m_Buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		break;
	}
	case BUFFER_INDEX: {
		// Bind index buffer
		vkCmdBindIndexBuffer(commandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		break;
	}
	}
}

// Find appropriate memory type
uint32_t Buffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	// Get memory properties
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_Device->GetPhysicalDevice(), &memProperties);

	// Find suitable memory type
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}
