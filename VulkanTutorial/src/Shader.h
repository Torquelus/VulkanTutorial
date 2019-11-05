#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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

	bool operator==(const Vertex& other) const {
		return position == other.position && colour == other.colour && texCoord == other.texCoord;
	}
};

// Create hash data for vertex
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.colour) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

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