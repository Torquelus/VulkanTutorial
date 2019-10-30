#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

#include "Shader.h"
#include "Buffer.h"

// CONST VARIABLES
const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

// Validation layers to use
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Device extensions to use
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Define if validation is enabled
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// STRUCTS
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

// Application Class
class Application {
public:
	Application();				// Constructor
	~Application();				// Destructor

	void Run();					// Run application
private:
	// VARIABLES
	GLFWwindow* m_Window;		// Main window
	VkInstance m_Instance;		// Vulkan instance
	VkSurfaceKHR m_Surface;		// Vulkan surface
	VkQueue m_PresentQueue;		// Vulkan presentation queue
	VkDevice m_Device;			// Vulkan logical device
	VkSwapchainKHR m_SwapChain;	// Vulkan swap chain
	VkRenderPass m_RenderPass;	// Vulkan render pass
	VkPipeline m_GraphicsPipeline;				// Vulkan graphics pipeline
	VkDescriptorSetLayout m_DescriptorSetLayout;// Vulkan descriptor set layout
	VkPipelineLayout m_PipelineLayout;			// Vulkan graphics pipeline layout
	VkCommandPool m_CommandPool;				// Vulkan command pool
	VkDescriptorPool m_DescriptorPool;			// Vulkan descriptor pool
	std::vector<VkDescriptorSet> m_DescriptorSets;	// Vulkan descriptor sets
	Buffer* m_VertexBuffer;						// Vertex buffer
	Buffer* m_IndexBuffer;						// Index buffer
	std::vector<Buffer*> m_UniformBuffers;		// Vector of uniform buffers
	std::vector<VkCommandBuffer> m_CommandBuffers;		// Vk command buffers
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;	// Vk framebuffers
	std::vector<VkImage> m_SwapChainImages;		// VkImages in swap chain
	std::vector<VkImageView> m_SwapChainImageViews;	// Vulkan image views
	VkFormat m_SwapChainImageFormat;			// Vulkan swap chain image format
	VkExtent2D m_SwapChainExtent;				// Vulkan swap chain extent
	VkPhysicalDevice m_PhysicalDevice;			// Vulkan physical device
	VkQueue m_GraphicsQueue;	// Vulkan graphics queue
	VkDebugUtilsMessengerEXT m_DebugMessenger;	// Vulkan debug logger
	size_t m_CurrentFrame;						// Frame index
	bool m_FramebufferResized = false;			// Bool for if screen has been resized

	// SEMAPHORES AND FRAMES
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;

	// FUNCTIONS
	void InitWindow();			// Initialise GLFW and Window
	void InitVulkan();			// Initialise Vulkan
	void CreateInstance();		// Create Vulkan instance
	void CreateSurface();		// Create Vulkan surface
	void CreateLogicalDevice();	// Create logical Vulkan device
	void CreateSwapChain();		// Create Vulkan swap chain
	void RecreateSwapChain();	// Recreate Vulkan swapchain (runtime)
	void CleanupSwapChain();	// Clean swap chain
	void CreateImageViews();	// Create Vulkan image views
	void CreateRenderPass();	// Create Vulkan rendering pass
	void CreateDescriptorSetLayout();	// Create descriptor set layout
	void CreateGraphicsPipeline();		// Create Vulkan graphics pipeline
	void CreateCommandPool();	// Create Vulkan command pool
	void CreateCommandBuffers();// Create command buffers for command pool
	void CreateFramebuffers();	// Create frame buffers
	void CreateSemaphores();	// Create semaphores
	void CreateVertexBuffer();	// Create vertex buffer
	void CreateIndexBuffer();	// Create index buffer
	void CreateUniformBuffers();// Create uniform buffers
	void CreateDescriptorPool();// Create descriptor pool
	void CreateDescriptorSets();// Create descriptor sets
	void DrawFrame();			// Draw frame with Vulkan
	void UpdateUniformBuffer(uint32_t currentImage);	// Update uniform buffer for rotation
	void PickPhysicalDevice();	// Select physical device for Vulkan to use
	void SetupDebugMessenger();	// Setup vulkan debug logger
	
	// ASSISTING FUNCTIONS
	int RateDeviceSuitable(VkPhysicalDevice device);		// Return suitability score of device
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);		// Returns true if extensions are supported
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);	// Return queue indices available on device
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);	// Query swap chains
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);	// Select appropriate format
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);		// Select appropriate presentation mode
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);								// Choose resolution of swap chain images
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);		// Create debug logger
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	bool CheckValidationLayerSupport();	// Check selected validation layers are supported by Vulkan
	std::vector<const char*> GetRequiredExtensions();	// Get list of enabled extensions
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);		// Debug call for debug logging
};