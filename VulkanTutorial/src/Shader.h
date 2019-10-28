#pragma once

#include <vector>
#include <string>
#include "vulkan/vulkan.h"

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