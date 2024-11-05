#ifndef VULKANINSTANCE
#define VULKANINSTANCE

#include "DisplayInstance.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <optional>

class VulkanInstance {
	public:
		VulkanInstance();
		void setupDevices(const DisplayInstance &displayInstance);
		void cleanup();

		VkInstance instance;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		
	private:
		const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation"};
		VkDebugUtilsMessengerEXT debugMessenger;

		void createInstance();
		void createLogicalDevice(const DisplayInstance &displayInstance);

		
		
		std::vector<const char *> getRequiredExtensions();

		void setupDebugMessenger();
		bool checkValidationLayerSupport();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
};

#endif
