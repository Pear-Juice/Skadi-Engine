#ifndef GLFWINSTANCE
#define GLFWINSTANCE

#include <cstdint>
#include <optional>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <string>

class DisplayInstance {
	public:
		DisplayInstance();

		struct QueueFamilyIndecies {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}

		};
		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		GLFWwindow *window;
		VkSurfaceKHR surface;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		QueueFamilyIndecies queueFamilyIndecies;
		SwapChainSupportDetails swapChainSupport;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		bool windowResized;
		uint32_t windowWidth;
		uint32_t windowHeight;

		void initDisplay(VkInstance &instance, uint32_t width, uint32_t height, std::string title);
		GLFWwindow* createWindow(uint32_t width, uint32_t height, std::string title, GLFWframebuffersizefun framebufferResizeCallback);
		VkSurfaceKHR createSurface(VkInstance &instance, GLFWwindow *window);

		void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

		bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		QueueFamilyIndecies findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		const std::vector<const char *> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDeviceProperties physicalDeviceProperties);
		void cleanup(VkInstance &instance);
	

	private:
		static void GLFWframebufferResizeCallback(GLFWwindow *window, int width, int height);
};

#endif
