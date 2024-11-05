#include "DisplayInstance.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <set>
#include <vulkan/vulkan_core.h>

DisplayInstance::DisplayInstance() {
	glfwInit();
}

void DisplayInstance::initDisplay(VkInstance &instance, uint32_t width, uint32_t height, std::string title) {
	window = createWindow(width, height, title, GLFWframebufferResizeCallback);
	surface = createSurface(instance, window);
	pickPhysicalDevice(instance, surface);
}

GLFWwindow* DisplayInstance::createWindow(uint32_t width, uint32_t height, std::string title, GLFWframebuffersizefun framebufferResizeCallback) {
	GLFWwindow* window;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	windowWidth = width;
	windowHeight = height;

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	return window;
}

void DisplayInstance::GLFWframebufferResizeCallback(GLFWwindow *window, int width, int height) {
	auto app = reinterpret_cast<DisplayInstance*>(glfwGetWindowUserPointer(window));	
	
	app->windowWidth = width;
	app->windowHeight = height;
	app->windowResized = true;
}

///creates the window surface
///Note: this surface is platform agnostic
VkSurfaceKHR DisplayInstance::createSurface(VkInstance &instance, GLFWwindow *window) {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface");

	return surface;
}

void DisplayInstance::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto &device : devices) {
		if (isDeviceSuitable(device, surface)) {
			vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount(physicalDeviceProperties);
			queueFamilyIndecies = findQueueFamilies(device, surface);
			swapChainSupport = querySwapChainSupport(device, surface);
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find suitable GPU");
	}
		
	std::cout << "Rendering with: " << physicalDeviceProperties.deviceName << physicalDevice << '\n';
}	

VkSampleCountFlagBits DisplayInstance::getMaxUsableSampleCount(VkPhysicalDeviceProperties physicalDeviceProperties) {
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	//Can this be replaaced with a switch? I dont know bit operations very well
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

///Checks physical device for required queue families, extension support, and swap chain adequacy 
bool DisplayInstance::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndecies indecies = findQueueFamilies(device, surface);
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indecies.isComplete() && extensionsSupported && swapChainAdequate;

	//VkPhysicalDeviceProperties2 deviceProperties;
	//VkPhysicalDeviceFeatures deviceFeatures;

	//vkGetPhysicalDeviceProperties2(device, &deviceProperties);
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	//return deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
}

///Checks if physical device supports swapchain and other extensions
bool DisplayInstance::checkDeviceExtensionSupport(VkPhysicalDevice device) { 
	uint32_t extensionsCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
	std::vector<VkExtensionProperties> availibleExtensions(extensionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availibleExtensions.data());
	
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto &extension : availibleExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

DisplayInstance::SwapChainSupportDetails DisplayInstance::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	
	//Populate swap chain device details with surface's capabilities
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	
	return details;
}

///Returns a QueueFamilyIndecies struct that contains the index of a graphics family and the inde of a present family 
///Notes: breaks early if a queue is found that supports both graphics and presentation 
DisplayInstance::QueueFamilyIndecies DisplayInstance::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndecies indecies;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto &queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indecies.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		

		if (presentSupport) {
			indecies.presentFamily = i;
		}

		if (indecies.isComplete())
			break;

		i++;
	}

	return indecies;
}


void DisplayInstance::cleanup(VkInstance &instance) {
	glfwDestroyWindow(window);
	vkDestroySurfaceKHR(instance, surface, nullptr);	

	glfwTerminate();
}
