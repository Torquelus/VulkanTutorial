#pragma once

#include <vector>
#include <string>
#include <array>

#include <glm/glm.hpp>
#include "vulkan/vulkan.h"

// Vertex struct
struct Vertex {
	glm::vec3 position;
	glm::vec3 colour;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, colour);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

// Our vertex data
const std::vector<Vertex> vertices = {
	{	{-0.5f, -0.5f, 0.0f},		{1.0f, 0.0f, 0.0f},		{1.0f, 0.0f}	},
	{	{0.5f, -0.5f, 0.0f},		{0.0f, 1.0f, 0.0f},		{0.0f, 0.0f}	},
	{	{0.5f,  0.5f, 0.0f},		{0.0f, 0.0f, 1.0f},		{0.0f, 1.0f}	},
	{	{-0.5f, 0.5f, 0.0f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f}	},

	{	{-0.5f, -0.5f, -0.5f},		{1.0f, 0.0f, 0.0f},		{1.0f, 0.0f}	},
	{	{0.5f, -0.5f, -0.5f},		{0.0f, 1.0f, 0.0f},		{0.0f, 0.0f}	},
	{	{0.5f,  0.5f, -0.5f},		{0.0f, 0.0f, 1.0f},		{0.0f, 1.0f}	},
	{	{-0.5f, 0.5f, -0.5f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f}	}
};

// Index data
const std::vector<uint16_t> indices = {
	0, 1, 2,
	2, 3, 0,

	4, 5, 6,
	6, 7, 4
};

// MVP uniform struct
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

// Shader class
class Shader {
public:
	Shader(VkDevice device, std::string vertpath, std::string fragpath);		// Constructor
	~Shader();		// Destructor

	// Getters
	VkShaderModule GetVertexShaderModule() { return m_VertexShaderModule; }
	VkShaderModule GetFragmentShaderModule() { return m_FragmentShaderModule; }
private:
	VkDevice m_Device;							// Vulkan logical device

	VkShaderModule m_VertexShaderModule;		// Vertex shader module
	VkShaderModule m_FragmentShaderModule;		// Fragment shader module

	static std::vector<char> ReadFile(const std::string& filename);		// Read shader code file into bytes
	VkShaderModule CreateShaderModule(const std::vector<char>& code);	// Create shader module from bytecode
};