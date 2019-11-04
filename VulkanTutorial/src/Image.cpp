#include "Image.h"

#include <stdexcept>

// Constructor
Image::Image(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
: m_Device(device), m_CommandPool(commandPool), m_Format(format) {

	// Image creation info
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = m_Format;
	imageInfo.tiling = tiling;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create image
	if (vkCreateImage(m_Device->GetDevice(), &imageInfo, nullptr, &m_Image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image!");
	}

	// Get memory requirements
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device->GetDevice(), m_Image, &memRequirements);

	// Memory allocation info
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	// Allocate memory
	if (vkAllocateMemory(m_Device->GetDevice(), &allocInfo, nullptr, &m_ImageMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory!");
	}

	// Bind image to image memory
	vkBindImageMemory(m_Device->GetDevice(), m_Image, m_ImageMemory, 0);

}

// Destructor
Image::~Image(){
	// Destroy image and free memory
	vkDestroyImage(m_Device->GetDevice(), m_Image, nullptr);
	vkFreeMemory(m_Device->GetDevice(), m_ImageMemory, nullptr);
}

// Copy a buffer of data to image
void Image::CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height) {
	// Create command
	VkCommandBuffer commandBuffer = m_CommandPool->BeginSingleTimeCommands();

	// Buffer image copy information
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	// Copy buffer to image
	vkCmdCopyBufferToImage(commandBuffer, buffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// End commands
	m_CommandPool->EndSingleTimeCommands(commandBuffer);
}

void Image::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {

	// Start command buffer
	VkCommandBuffer commandBuffer = m_CommandPool->BeginSingleTimeCommands();

	// Image memory barrier info
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_Image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	// Check which aspect to use
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		// Check for stencil attachment
		if (m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_Format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage, destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("Unsupported layout transition!");
	}

	// Pipeline barrier command
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	// End command buffer
	m_CommandPool->EndSingleTimeCommands(commandBuffer);

}

// Find required memory type
uint32_t Image::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
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
