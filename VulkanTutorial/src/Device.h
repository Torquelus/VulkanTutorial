#pragma once

#include "vulkan/vulkan.h"

#include <optional>
#include <vector>

// Define if validation is enabled
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Validation layers to use
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// STRUCTS
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

// Device extensions to use
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Device {
public:
	Device(VkInstance instance, VkSurfaceKHR surface);		// Constructor
	~Device();		// Destructor

	// FUNCTIONS
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);	// Query swap chains
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);			// Return queue indices available on device

	// GETTERS
	VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
	VkDevice GetDevice() { return m_Device; }
	VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
	VkQueue GetPresentQueue() { return m_PresentQueue; }
	VkSampleCountFlagBits GetSamples() { return m_MsaaSamples; }
private:
	// VARIABLES
	VkPhysicalDevice m_PhysicalDevice;		// Vulkan physical device
	VkDevice m_Device;						// Vulkan logical device
	VkQueue m_GraphicsQueue;				// Vulkan graphics queue
	VkQueue m_PresentQueue;					// Vulkan present queue
	VkSurfaceKHR m_Surface;					// Vulkan surface
	VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;	// MSAA samples

	// FUNCTIONS
	void CreateLogicalDevice();						// Create Vulkan logical devic
	void PickPhysicalDevice(VkInstance instance);	// Select physical device for Vulkan to use
	int RateDeviceSuitable(VkPhysicalDevice device);						// Return suitability score of device
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);				// Returns true if extensions are supported
	VkSampleCountFlagBits GetMaxUsableSampleCount();	// Max MSAA samples amount

};