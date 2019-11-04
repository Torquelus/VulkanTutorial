#pragma once

#include "vulkan/vulkan.h"
#include "Device.h"

class CommandPool {
public:
	CommandPool(Device* device);		// Constructor
	~CommandPool();						// Destructor

	// FUNCTIONS
	VkCommandBuffer BeginSingleTimeCommands();					// Begin single time command
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);	// End signle time commands

	// GETTERS
	VkCommandPool GetCommandPool() { return m_CommandPool; }

private:
	// VARIABLES
	VkCommandPool m_CommandPool;		// Vulkan command pool
	Device* m_Device;					// Vulkan device	
};