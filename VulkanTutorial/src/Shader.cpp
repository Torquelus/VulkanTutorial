#include "Shader.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

// Constructor
Shader::Shader(VkDevice device, std::string vertpath, std::string fragpath) : m_Device(device) {

	// Get vertex and fragment shader codes
	auto vertexShaderCode = ReadFile(vertpath);
	auto fragmentShaderCode = ReadFile(fragpath);

	// Create shader modules from bytecode
	m_VertexShaderModule = CreateShaderModule(vertexShaderCode);
	m_FragmentShaderModule = CreateShaderModule(fragmentShaderCode);

}

// Destructor
Shader::~Shader() {

	// Destroy shader modules
	vkDestroyShaderModule(m_Device, m_FragmentShaderModule, nullptr);
	vkDestroyShaderModule(m_Device, m_VertexShaderModule, nullptr);

}

// Read shader code file into bytes
std::vector<char> Shader::ReadFile(const std::string& filename){
	// Open file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	// Check if file didn't open
	if (!file.is_open()) {
		std::cerr << "Failed to open file " << filename << std::endl;
	}

	// Get size
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Read all bytes of file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// Close file and return bytes
	file.close();
	return buffer;

}

// Create shader module from bytecode
VkShaderModule Shader::CreateShaderModule(const std::vector<char>& code){
	
	// Shader module creation info
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	// Create module
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	// Return created shader module
	return shaderModule;
}
