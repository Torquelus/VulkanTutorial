#include "Texture.h"

#include "stb/stb_image.h"
#include "glm/glm.hpp"

#include <stdexcept>
#include <algorithm>

// Constructor
Texture::Texture(Device* device, CommandPool* commandPool, const char* path)
	: m_Device(device) {
	// Image details
	int width, height, channels;

	// Load image
	stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

	// Get texture size
	VkDeviceSize imageSize = static_cast<double>(width) * static_cast<double>(height) * 4;

	// Check if image was loaded successfully
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image");
	}

	// Create staging buffer
	Buffer stagingBuffer(m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	// Map data to staging buffer
	void* data;
	vkMapMemory(m_Device->GetDevice(), stagingBuffer.GetBufferMemory(), 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_Device->GetDevice(), stagingBuffer.GetBufferMemory());

	// Clean up pixel array
	stbi_image_free(pixels);

	// Get mip levels
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	// Create image
	m_Image = new Image(m_Device, commandPool, static_cast<uint32_t>(width), static_cast<uint32_t>(height), mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	
	// Transition to new layout
	m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	m_Image->CopyBufferToImage(stagingBuffer.GetBuffer(), static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	//m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
	
	// Generate mipmaps for image
	m_Image->GenerateMipmaps();

}

// Destructor
Texture::~Texture(){
	// Delete image
	delete(m_Image);
}
