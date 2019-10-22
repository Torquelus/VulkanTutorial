#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

// CONST VARIABLES
const int WIDTH = 800;
const int HEIGHT = 600;

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
	VkPhysicalDevice m_PhysicalDevice;			// Vulkan physical device
	VkQueue m_GraphicsQueue;	// Vulkan graphics queue
	VkDebugUtilsMessengerEXT m_DebugMessenger;	// Vulkan debug logger

	// FUNCTIONS
	void InitWindow();			// Initialise GLFW and Window
	void InitVulkan();			// Initialise Vulkan
	void CreateInstance();		// Create Vulkan instance
	void CreateSurface();		// Create Vulkan surface
	void CreateLogicalDevice();	// Create logical Vulkan device
	void PickPhysicalDevice();	// Select physical device for Vulkan to use
	void SetupDebugMessenger();	// Setup vulkan debug logger
	
	// ASSISTING FUNCTIONS
	int RateDeviceSuitable(VkPhysicalDevice device);		// Return suitability score of device
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);		// Returns true if extensions are supported
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);	// Return queue indices available on device
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