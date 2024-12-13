#ifndef REND
#define REND

#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <Dependancies/uuid.h>

#include "Source/Input/Input.hpp"
#include "Source/Resources/Texture.hpp"
#include "Source/Resources/Model.hpp"
#include "Source/Resources/Material.hpp"
#include "UniformBufferObject.hpp"
#include "ResourceManager.hpp"
#include "VulkMesh.hpp"
#include "VulkTexture.hpp"
#include "VulkMaterial.hpp"
#include <uuid/uuid.h>

class Rend {
	public:
		void beginLoop();
		void initVulkan();
		void registerMaterial(Material &material);
		void renderModel(Model model);
		void processModelQueue();

        DisplayInstance *displayInstance;
        VulkanInstance *vInstance;
        ResourceManager *resourceManager;

		Rend();

	private:
		std::mutex submitMutex;
		std::queue<Model> modelQueue;
		std::unordered_map<uuids::uuid, VulkMesh> vulkMeshes;
		std::unordered_map<uuids::uuid, VulkMaterial> vulkMaterials;


		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkFramebuffer> swapChainFramebuffers;
		
		VkRenderPass renderPass;

		VkDescriptorSetLayout uboDescriptorSetLayout;
		VkDescriptorPool uboDescriptorPool;
		std::vector<VkDescriptorSet> uboDescriptorSets;

	//TEMP
		VkDescriptorSetLayout samplerDescriptorSetLayout;
		ResourceManager::GraphicsPipeline graphicsPipeline;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;



		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		//VkBuffer stagingBuffer;
		//VkDeviceMemory stagingBufferMemory;
		//uint32_t mipLevels;
		//VkImage textureImage;
		//VkDeviceMemory textureImageMemory;
		//VkImageView textureImageView;
		//VkSampler textureSampler;

		//VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		int frame = 0;

		void mainLoop();
		void cleanup();
		void cleanupVulkMaterial(VulkMaterial material);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createSwapChain();
		void cleanupSwapChain();
		void recreateSwapChain();
		void createImageViews();
		void createFramebuffers();
		VkFormat findSupportedImageFormat(const std::vector<VkFormat>&candidates, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
		VkFormat findDepthFormat();
		bool hasStencilComponent(VkFormat format);
		void createDepthResources();
		void createColorResources();

		VulkTexture createVulkTexture(Texture tex);
		void updateImage(Texture tex, VulkTexture &vulkTexture);
		void updateSampler(VkPhysicalDeviceProperties physicalDeviceProperties, VulkTexture &vulkTexture);
		void updateImageView(VulkTexture &vulkTexture);
		
		void generateMipMaps(VkImage image, VkFormat imageFormat, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels);	
		void createCommandBuffers();
		void createUniformBuffers();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void createRenderPass();
		void createSyncObjects();
		void drawFrame();
		void updateUniformBuffer(UniformBufferObject ubo);
};

#endif
