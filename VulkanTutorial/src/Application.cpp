#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Constructor
Application::Application() {
	InitWindow();
	InitVulkan();
}

// Destructor
Application::~Application() {

	// Clean up swapchain
	CleanupSwapChain();

	// Destroy sampler
	vkDestroySampler(m_Device->GetDevice(), m_TextureSampler, nullptr);

	// Destroy descriptor set layout
	vkDestroyDescriptorSetLayout(m_Device->GetDevice(), m_DescriptorSetLayout, nullptr);

	// Delete model
	delete(m_Model);

	// Destroy semaphores
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_Device->GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_Device->GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_Device->GetDevice(), m_InFlightFences[i], nullptr);
	}

	// Destroy command pool
	delete(m_CommandPool);

	// Delete device
	delete(m_Device);

	// Delete debug messenger if enabled
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}

	// Destroy vulkan surface
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

	// Destroy vulkan instance
	vkDestroyInstance(m_Instance, nullptr);

	// Destroy window
	glfwDestroyWindow(m_Window);

	// Terminate GLFW
	glfwTerminate();
}

// Run application
void Application::Run() {

	// Loop while not closing window
	while (!glfwWindowShouldClose(m_Window)) {
		// Poll for events
		glfwPollEvents();
		// Draw frame
		DrawFrame();
	}

	// Wait for device to finish before exiting
	vkDeviceWaitIdle(m_Device->GetDevice());

}

// Initialise GLFW and window
void Application::InitWindow() {
	// Initialise GLFW
	if (glfwInit() == GLFW_FALSE) {
		throw std::runtime_error("Failed to initialise GLFW!");
	}

	// Telling GLFW not to use OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// Create window
	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);

	// Set callback
	glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
}

// Initialise Vulkan
void Application::InitVulkan() {
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	CreateDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateCommandPool();
	CreateDepthResources();
	CreateFramebuffers();
	CreateTextureSampler();
	LoadModel();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSemaphores();
}

// Create vulkan instance
void Application::CreateInstance() {
	// Check validation layers
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("Some validation layers were requested but were not available!");
	}

	// Fill in application info
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Fill in extensions and validation layers to use
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	// Get extensions
	auto extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Include validation layers if enabled, otherwise set enabled layers count to 0
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Instantiate vulkan
	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance!");
	}
}

// Create Vulkan surface
void Application::CreateSurface(){
	if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
}

// Create device
void Application::CreateDevice(){
	m_Device = new Device(m_Instance, m_Surface);
}

// Create Vulkan swap chain
void Application::CreateSwapChain(){
	// Query swap chain support details from physical device
	SwapChainSupportDetails swapChainSupport = m_Device->QuerySwapChainSupport(m_Device->GetPhysicalDevice());

	// Choose desired swap chain settings from supported formats
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	// Number of frames to request
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Swap chain creation info
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Query queue families for swap chain info
	QueueFamilyIndices indices = m_Device->FindQueueFamilies(m_Device->GetPhysicalDevice());
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	// Continue swap chain creation info
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create swap chain
	if (vkCreateSwapchainKHR(m_Device->GetDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}

	// Fill swapchain images vector
	vkGetSwapchainImagesKHR(m_Device->GetDevice(), m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device->GetDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

	// Assign member variables
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;
}

// Recreate Vulkan swapchain (runtime)
void Application::RecreateSwapChain() {
	// Check if minimized
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}

	// Wait for device to be unused
	vkDeviceWaitIdle(m_Device->GetDevice());

	// Clean up previous swapchain
	CleanupSwapChain();

	// Recreate swapchain and everything
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateDepthResources();
	CreateFramebuffers();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
}

// Clean swap chain
void Application::CleanupSwapChain(){
	// Delete depth image and imageview
	delete(m_DepthImageView);
	delete(m_DepthImage);
	
	// Destroy framebuffers
	for (auto framebuffer : m_SwapChainFramebuffers) {
		vkDestroyFramebuffer(m_Device->GetDevice(), framebuffer, nullptr);
	}

	// Free command buffers
	vkFreeCommandBuffers(m_Device->GetDevice(), m_CommandPool->GetCommandPool(), static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());

	// Destroy pipeline
	vkDestroyPipeline(m_Device->GetDevice(), m_GraphicsPipeline, nullptr);

	// Destroy pipeline layout
	vkDestroyPipelineLayout(m_Device->GetDevice(), m_PipelineLayout, nullptr);

	// Destroy render pass
	vkDestroyRenderPass(m_Device->GetDevice(), m_RenderPass, nullptr);

	// Destroy all image views
	for (auto imageView : m_SwapChainImageViews) {
		delete (imageView);
	}

	// Destroy swap chain
	vkDestroySwapchainKHR(m_Device->GetDevice(), m_SwapChain, nullptr);

	// Destroy uniform buffers
	for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
		delete(m_UniformBuffers[i]);
	}

	// Destroy descriptor pool
	vkDestroyDescriptorPool(m_Device->GetDevice(), m_DescriptorPool, nullptr);
}

// Create Vulkan image views
void Application::CreateImageViews() {
	// Resize to fit all images
	m_SwapChainImageViews.resize(m_SwapChainImages.size());

	// Loop through all images
	for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
		m_SwapChainImageViews[i] = new ImageView(m_Device, m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

// Create Vulkan rendering pass
void Application::CreateRenderPass(){

	// Vulkan colour attachment description
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_SwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Colour attachment reference
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Vulkan depth attachment description
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Depth attachment reference
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Subpass references
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// Create subpass dependencies
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	
	// Render pass creation info
	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	
	// Create render pass
	if (vkCreateRenderPass(m_Device->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

// Create descriptor set layout
void Application::CreateDescriptorSetLayout(){

	// Descriptor binding info
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	// Descriptor for sampler
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Descriptor set layout creation info
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	
	// Create descriptor set layout
	if (vkCreateDescriptorSetLayout(m_Device->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}

}

// Create Vulkan graphics pipeline
void Application::CreateGraphicsPipeline(){	
	// Create shader
	Shader shader(m_Device->GetDevice(), "src/res/shaders/vert.spv", "src/res/shaders/frag.spv");

	// Vertex shader stage creation info
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = shader.GetVertexShaderModule();
	vertShaderStageInfo.pName = "main";

	// Fragment shader stage creation info
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = shader.GetFragmentShaderModule();
	fragShaderStageInfo.pName = "main";

	// Put info into array
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input creation info
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input assembly creation info
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport details
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_SwapChainExtent.width;
	viewport.height = (float)m_SwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor details
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;

	// Viewport state creation info
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer creation info
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// Multisampling info
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Depth stencil info
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	// Colour blending info
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Colour blending state info
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Dynamic state of pipeline
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// Pipeline layout creation info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	// Create pipeline layout
	if (vkCreatePipelineLayout(m_Device->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	// Graphics pipeline creation info
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	// Create pipeline
	if (vkCreateGraphicsPipelines(m_Device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

}

// Create Vulkan command pool
void Application::CreateCommandPool(){

	m_CommandPool = new CommandPool(m_Device);

}

// Create and allocate resources for depth buffering
void Application::CreateDepthResources(){

	// Find depth format
	VkFormat depthFormat = FindDepthFormat();

	// Create image
	m_DepthImage = new Image(m_Device, m_CommandPool, m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Create depth image view
	m_DepthImageView = new ImageView(m_Device, m_DepthImage->GetImage(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	// Transition depth image to attachment
	m_DepthImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

// Create command buffers for command pool
void Application::CreateCommandBuffers(){
	// Resize to fit all framebuffers
	m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

	// Command buffer allocation info
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool->GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

	// Allocate command buffers
	if (vkAllocateCommandBuffers(m_Device->GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	// Start command buffers
	for (size_t i = 0; i < m_CommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		// Begin buffer
		if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		// Render pass begin info
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainExtent;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin render pass
		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind graphics pipeline
		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		// Bind model
		m_Model->Bind(m_CommandBuffers[i]);

		// Bind descriptor sets
		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

		// Draw model
		m_Model->Draw(m_CommandBuffers[i]);

		// End render pass
		vkCmdEndRenderPass(m_CommandBuffers[i]);

		// End recording
		if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}

	
}

// Create frame buffers
void Application::CreateFramebuffers(){
	// Resize container to hold all framebuffers
	m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

	// Loop through image views
	for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
		// Create attachments
		std::array<VkImageView, 2> attachments = { m_SwapChainImageViews[i]->GetImageView(), m_DepthImageView->GetImageView() };

		// Framebuffer creation info
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_SwapChainExtent.width;
		framebufferInfo.height = m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		// Create framebuffer for each image view
		if (vkCreateFramebuffer(m_Device->GetDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

// Create semaphores
void Application::CreateSemaphores(){
	// Resize semaphores vector to fit a semaphore for every concurrent frame
	m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	// Semaphore creation info
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation info
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// Create semaphores for each frame
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(m_Device->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image available semaphore for a frame!");
		}
		if (vkCreateSemaphore(m_Device->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render finished semaphore for a frame!");
		}
		if (vkCreateFence(m_Device->GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create fence for a frame!");
		}
	}

	// Initialise currentframe
	m_CurrentFrame = 0;
}

// Create texture sampler
void Application::CreateTextureSampler(){

	// Sampler creation info
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	// Create texture sampler
	if (vkCreateSampler(m_Device->GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler!");
	}

}

// Load in obj model
void Application::LoadModel(){
	m_Model = new Model(m_Device, m_CommandPool, "src/res/models/test.obj", "src/res/textures/n64.png");
}

// Create uniform buffers
void Application::CreateUniformBuffers(){

	// Get buffer size and resize vector
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	m_UniformBuffers.resize(m_SwapChainImages.size());

	// Create uniform buffer for each swapchainimage
	for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
		m_UniformBuffers[i] = new Buffer(m_Device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, BUFFER_UNIFORM);
	}

}

// Create descriptor pool
void Application::CreateDescriptorPool(){

	// Pool size description
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());

	// Descriptor pool creation info
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size());

	// Create descriptor pool
	if (vkCreateDescriptorPool(m_Device->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}

}

// Create descriptor sets
void Application::CreateDescriptorSets(){

	// Create set allocations
	std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	// Resize set and allocate
	m_DescriptorSets.resize(m_SwapChainImages.size());
	if (vkAllocateDescriptorSets(m_Device->GetDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	// Populate descriptor set
	for (size_t i = 0; i < m_SwapChainImages.size(); i++) {

		// Buffer info
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_UniformBuffers[i]->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		// Descriptor image info
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_Model->GetTextureView();
		imageInfo.sampler = m_TextureSampler;

		// Descriptor set update info
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_DescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		// update descriptor sets
		vkUpdateDescriptorSets(m_Device->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	}

}

// Draw frame with Vulkan
void Application::DrawFrame(){
	// Wait for frame to be finished
	vkWaitForFences(m_Device->GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	// Aquire next image
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Device->GetDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	// Check if swapchain no longer compatible
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	// Update uniform buffer
	UpdateUniformBuffer(imageIndex);

	// Queue submission info
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];
	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Reset fence and submit to queue
	vkResetFences(m_Device->GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
	if (vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	// Presentation info
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	// Present queue
	result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &presentInfo);

	// Check again for swap chain
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
		m_FramebufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image");
	}

	// Advance to next frame
	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// Update uniform buffer for rotation
void Application::UpdateUniformBuffer(uint32_t currentImage){
	// Get start time of program
	static auto startTime = std::chrono::high_resolution_clock::now();

	// Get current time
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// Create object and rotate
	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChainExtent.width / (float)m_SwapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	// Copy data to uniform buffer
	void* data;
	vkMapMemory(m_Device->GetDevice(), m_UniformBuffers[currentImage]->GetBufferMemory(), 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(m_Device->GetDevice(), m_UniformBuffers[currentImage]->GetBufferMemory());
}

// Setup debug logger
void Application::SetupDebugMessenger(){
	// Do nothing if validation layers disabled
	if (!enableValidationLayers) return;

	// Creation info
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	// Which severities to enabled: verbose, warning, and error
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	// Which types of messages to enable: general, validation, and performance
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	// Pointer to callback function
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;

	// Create messenger
	if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

// Select appropriate format
VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
	
	// Find preferable colour channels
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// Else return first format
	return availableFormats[0];
}

// Select appropriate presentation mode
VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
	
	// Look for preferable presentation mode
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	// Else return guaranteed mode
	return VK_PRESENT_MODE_FIFO_KHR;
}

// Choose resolution of swap chain images
VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
	
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);

		VkExtent2D actualExtent = { 
			static_cast<uint32_t>(width), 
			static_cast<uint32_t>(height)
		};
		
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}

}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height){
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->m_FramebufferResized = true;
}

VkCommandBuffer Application::BeginSingleTimeCommands(){
	// Command buffer allocation info
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool->GetCommandPool();
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

void Application::EndSingleTimeCommands(VkCommandBuffer commandBuffer){
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
	vkFreeCommandBuffers(m_Device->GetDevice(), m_CommandPool->GetCommandPool(), 1, &commandBuffer);
}

// Find supported image formats
VkFormat Application::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
	
	// Loop through all format candidates
	for (VkFormat format : candidates) {
		// Query properties
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_Device->GetPhysicalDevice(), format, &props);

		// Check for tiling
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	// If not suitable format found throw error
	throw std::runtime_error("Failed to find supported format!");

}

// Find suitable format for depth image
VkFormat Application::FindDepthFormat(){
	return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

// Check if format has stencil component
bool Application::HasStencilComponent(VkFormat format){
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

// Create debug logger
VkResult Application::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

// Destroy debug logger
void Application::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


// Check selected validation layers are supported by Vulkan
bool Application::CheckValidationLayerSupport() {
	uint32_t layerCount;
	// Find number of available layers
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Get all available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Loop through all chosen layers
	for (const char* layerName : validationLayers) {
		// Flag if chosen layer was found in list of available layers
		bool layerFound = false;

		// Loop through all available layers
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		// If layer wasn't found exit function with false
		if (!layerFound) {
			return false;
		}
	}

	// Return true if all was found
	return true;
}

// Get list of enabled extensions
std::vector<const char*> Application::GetRequiredExtensions(){
	// Glfw extensions count and list
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	// Get glfw extension count and extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Make vector from extensions
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	
	// Add debug utils to extensions if debug mode
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Return vector of extensions
	return extensions;
}

// Debug call for debug logging
VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
	
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}
