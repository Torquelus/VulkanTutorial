#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "vendor/tinyobjloader/tiny_obj_loader.h"

#include <unordered_map>

// Constructor
Model::Model(Device* device, CommandPool* commandPool, const char* modelPath, const char* texturePath) 
	: m_Device(device), m_CommandPool(commandPool) {
	// Load texture
	m_Texture = new Texture(m_Device, m_CommandPool, texturePath);
	
	// Objects to load model into
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	// Load object
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath)) {
		throw std::runtime_error(warn + err);
	}

	// Map for unique vertices
	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	// Loop through shapes
	for (const auto& shape : shapes) {
		// Loop through all indices
		for (const auto& index : shape.mesh.indices) {
			// Create vertex
			Vertex vertex = {};

			// Set vertex details
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			
			vertex.colour = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				// Create unique vertex and push to vertices
				uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
				m_Vertices.push_back(vertex);
			}
			
			m_Indices.push_back(uniqueVertices[vertex]);
		}
	}

	CreateVertexBuffer();
	CreateIndexBuffer();

}

// Destructor
Model::~Model(){
	// Delete texture
	delete(m_Texture);

	// Delete buffers
	delete(m_VertexBuffer);
	delete(m_IndexBuffer);
}

// Bind buffers
void Model::Bind(VkCommandBuffer commandBuffer){
	// Bind vertex buffer
	m_VertexBuffer->Bind(commandBuffer);

	// Bind index buffer
	m_IndexBuffer->Bind(commandBuffer);
}

// Draw model
void Model::Draw(VkCommandBuffer commandBuffer){
	// Draw triangle
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
}

// Create vertex buffer
void Model::CreateVertexBuffer() {

	// Get buffer size
	VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

	// Create staging buffer
	Buffer stagingBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map data
	void* data;
	vkMapMemory(m_Device->GetDevice(), stagingBuffer.GetBufferMemory(), 0, bufferSize, 0, &data);
	memcpy(data, m_Vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Device->GetDevice(), stagingBuffer.GetBufferMemory());

	// Create vertex buffer
	m_VertexBuffer = new Buffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, BUFFER_VERTEX);

	// Copy data to vertex buffer
	m_VertexBuffer->CopyToBuffer(m_CommandPool->GetCommandPool(), stagingBuffer.GetBuffer(), bufferSize);
}

// Create index buffer
void Model::CreateIndexBuffer() {

	// Get buffer size
	VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

	// Create staging buffer
	Buffer stagingBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map data
	void* data;
	vkMapMemory(m_Device->GetDevice(), stagingBuffer.GetBufferMemory(), 0, bufferSize, 0, &data);
	memcpy(data, m_Indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Device->GetDevice(), stagingBuffer.GetBufferMemory());

	// Create index buffer
	m_IndexBuffer = new Buffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, BUFFER_INDEX);

	// Copy data to index buffer
	m_IndexBuffer->CopyToBuffer(m_CommandPool->GetCommandPool(), stagingBuffer.GetBuffer(), bufferSize);

}