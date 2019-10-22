#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <map>
#include <set>

// Constructor
Application::Application() {
	InitWindow();
	InitVulkan();
}

// Destructor
Application::~Application() {
	// Destroy device
	vkDestroyDevice(m_Device, nullptr);

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
	}

}

// Initialise GLFW and window
void Application::InitWindow() {
	// Initialise GLFW
	if (glfwInit() == GLFW_FALSE) {
		throw std::runtime_error("Failed to initialise GLFW!");
	}

	// Telling GLFW not to use OpenGL and make window not resizable (for now)
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create window
	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

// Initialise Vulkan
void Application::InitVulkan() {
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
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

// Create logical Vulkan device
void Application::CreateLogicalDevice(){

	// Get queue data from physical device
	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

	// Create device queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
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
void Application::PickPhysicalDevice(){
	// Initialise physical device to null
	m_PhysicalDevice = VK_NULL_HANDLE;

	// Query number of physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

	// Exit if 0 devices found
	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	// Create and fill vector of device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

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
	}
	else {
		throw std::runtime_error("Failed to find suitable GPU!");
	}

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

// Return suitability score of device
int Application::RateDeviceSuitable(VkPhysicalDevice device){
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

	// Return score
	return score;
}

// Returns true if extensions are supported
bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device){
	
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

// Return queue indices available on device
QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device){
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

VkResult Application::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

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
