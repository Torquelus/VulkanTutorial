#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>

// CONST VARIABLES
const int WIDTH = 800;
const int HEIGHT = 600;

// Application Class
class HelloTriangleApplication {
public:
	// Run application
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	// VARIABLES
	GLFWwindow* window;			// Main window variable
	VkInstance instance;

	// FUNCTIONS
	// Initialise GLFW and Window
	void initWindow() {
		// Initialise GLFW
		if (glfwInit() == GLFW_FALSE) {
			throw std::runtime_error("Failed to initialise GLFW!");
		}

		// Telling GLFW not to use OpenGL and make window not resizable (for now)
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Create window
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	// Initialise Vulkan
	void initVulkan() {
		createInstance();
	}

	// Create vulkan instance
	void createInstance() {
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

		// Get number of glfw extensions in use
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Apply to createInfo
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		// Instantiate vulkan
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan instance!");
		}
	}

	// Main application loop
	void mainLoop() {
		// Loop while not closing window
		while (!glfwWindowShouldClose(window)) {
			// Poll for events
			glfwPollEvents();
		}
	}

	// Clean up all memory
	void cleanup() {
		// Destroy vulkan instance
		vkDestroyInstance(instance, nullptr);

		// Destroy window
		glfwDestroyWindow(window);

		// Terminate GLFW
		glfwTerminate();
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}