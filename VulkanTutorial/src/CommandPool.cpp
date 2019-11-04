#include "CommandPool.h"

#include <stdexcept>

// Constructor
CommandPool::CommandPool(Device* device) : m_Device(device) {
	// Command pool creation info
	QueueFamilyIndices queueFamilyIndices = m_Device->FindQueueFamilies(m_Device->GetPhysicalDevice());
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0;

	// Create command pool
	if (vkCreateCommandPool(m_Device->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

// Destructor
CommandPool::~CommandPool() {
	// Destroy command pool
	vkDestroyCommandPool(m_Device->GetDevice(), m_CommandPool, nullptr);
}

// Begin single time command
VkCommandBuffer CommandPool::BeginSingleTimeCommands() {
	// Command buffer allocation info
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_Device->GetDevice(), &allocInfo, &commandBuffer);

	// Begin info
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Begin command buffer
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Return command buffer
	return commandBuffer;
}

// End signle time commands
void CommandPool::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	// End command buffer
	vkEndCommandBuffer(commandBuffer);

	// Command buffer submit info
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Submit to queue
	vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_Device->GetGraphicsQueue());

	// Free command buffer
	vkFreeCommandBuffers(m_Device->GetDevice(), m_CommandPool, 1, &commandBuffer);
}