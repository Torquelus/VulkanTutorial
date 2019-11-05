#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "Buffer.h"
#include "ImageView.h"
#include "Shader.h"
#include "Texture.h"

class Model {
public:
	Model(Device* device, CommandPool* commandPool, const char* modelPath, const char* texturePath);	// Constructor
	~Model();	// Destructor

	// FUNCTIONS
	void Bind(VkCommandBuffer commandBuffer);	// Bind buffers
	void Draw(VkCommandBuffer commandBuffer);	// Draw model

	// GETTERS
	VkImageView GetTextureView() { return m_TextureView->GetImageView(); }
private:
	// VARIABLES
	Device* m_Device;				// Vulkan device
	CommandPool* m_CommandPool;		// Vulkan command pool
	std::vector<Vertex> m_Vertices;	// Vector of vertices
	std::vector<uint32_t> m_Indices;// Vector of indices
	Texture* m_Texture;				// Texture of model
	ImageView* m_TextureView;		// Image view of texture
	Buffer* m_VertexBuffer;			// Vertex buffer for model
	Buffer* m_IndexBuffer;			// Vertex buffer for model

	// FUNCTIONS
	void CreateVertexBuffer();		// Create vertex buffer
	void CreateIndexBuffer();		// Create index buffer
};