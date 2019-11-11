#include "Device.h"

#include <stdexcept>
#include <map>
#include <set>
#include <string>
#include <algorithm>

// Constructor
Device::Device(VkInstance instance, VkSurfaceKHR surface) : m_Surface(surface) {
	PickPhysicalDevice(instance);
	CreateLogicalDevice();
}

// Destructor
Device::~Device() {
	// Destroy device
	vkDestroyDevice(m_Device, nullptr);
}

// Create logical Vulkan device
void Device::CreateLogicalDevice() {

	// Get queue data from physical device
	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

	// Create device queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		// Create device queue
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Device features to use
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// Logical device creation info
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Create device
	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	// Get queue and store in m_GraphicsQueue
	vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
}

// Select physical device for Vulkan to use
void Device::PickPhysicalDevice(VkInstance instance) {
	// Initialise physical device to null
	m_PhysicalDevice = VK_NULL_HANDLE;

	// Query number of physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Exit if 0 devices found
	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	// Create and fill vector of device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Map to sort devices by score
	std::multimap<int, VkPhysicalDevice> candidates;

	// Check if any device meets requirements
	for (const auto& device : devices) {
		int score = RateDeviceSuitable(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check if best candidate is suitable else error
	if (candidates.rbegin()->first > 0) {
		m_PhysicalDevice = candidates.rbegin()->second;
		m_MsaaSamples = GetMaxUsableSampleCount();
	}
	else {
		throw std::runtime_error("Failed to find suitable GPU!");
	}

}

// Return suitability score of device
int Device::RateDeviceSuitable(VkPhysicalDevice device) {
	// Query device properties
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Query device features
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Score variable
	int score = 0;

	// Add score for discrete GPU
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Add maximum size of textures
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	// Check if queue is all good
	QueueFamilyIndices indices = FindQueueFamilies(device);
	if (!indices.IsComplete()) {
		return 0;
	}

	// Check extensions
	if (!CheckDeviceExtensionSupport(device)) {
		return 0;
	}

	// Check swap chain support
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
		return 0;
	}

	// Return score
	return score;
}

// Return queue indices available on device
QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	// Query queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Check if queue that supports graphics bit exists
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Check if queue supports presenting to window surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		// Break if queue is found
		if (indices.IsComplete()) {
			break;
		}

		i++;
	}


	// Return
	return indices;
}

// Returns true if extensions are supported
bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device) {

	// Query extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// Required extensions
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	// Loop through extensions
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	// return true if all required extensions were found
	return requiredExtensions.empty();

}

// Max MSAA samples amount
VkSampleCountFlagBits Device::GetMaxUsableSampleCount() {
	// Get physical device properties
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

	// Check colour and depth sampling count
	VkSampleCountFlags counts = std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);

	// Check amound of samples, else return 1 sample
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}

// Query swap chains
SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	// Query capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

	// Query suported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
	}

	// Query suported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
	}

	// Return swap chain support details
	return details;
}