#ifndef RESOURCEMANAGER
#define RESOURCEMANAGER

#include "DisplayInstance.hpp"
#include "Vertex.hpp"
#include "VulkanInstance.hpp"
#include <cstdint>
#include <mutex>
#include <queue>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <Source/Resources/Model.hpp>
#include <vulkan/vulkan_core.h>

#include "TransferBuffer.hpp"
#include "UniformBufferObject.hpp"
#include "VulkMesh.hpp"

#include "VulkTexture.hpp"

class ResourceManager {
	public:
		ResourceManager(const VulkanInstance &vulkanInstance, const DisplayInstance &displayInstance);

		struct GraphicsPipeline {
			VkPipeline pipeline;
			VkPipelineLayout layout;
		};

		VkDescriptorPool createDescriptorPool(uint32_t frameCount, uint32_t uboCount, uint32_t samplerCount);
		VkDescriptorSetLayout createDescriptorSetLayout(uint32_t uboCount, uint32_t fragSamplerCount, uint32_t vertSamplerCount);
        std::vector<VkDescriptorSet> createUBODescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, const std::vector<VkBuffer> &uniformBuffers, uint32_t frameCount);
        std::vector<VkDescriptorSet> createImageDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VulkTexture> textures, uint32_t frameCount);

		GraphicsPipeline createGraphicsPipeline(VkShaderModule vertShader, VkShaderModule fragShader, VkExtent2D windowExtent, VkRenderPass renderPass, VkSampleCountFlagBits , std::vector<VkDescriptorSetLayout> descriptorLayouts, std::vector<VkPushConstantRange> pushConstantRanges) const;
        VkShaderModule createShaderModule(const std::vector<char>& code);

		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

		TransferBuffer createTransferBuffer(VkFlags usageFlags, VkDeviceSize capacity);
        template<typename T> void transferBufferWrite(TransferBuffer &transferBuffer, std::vector<T> inputData) {
            transferBuffer.objectCount = inputData.size();

            void *data;

            vkMapMemory(device, transferBuffer.stagingBufferMemory, 0, transferBuffer.capacity, 0, &data);
            memcpy(data, inputData.data(), transferBuffer.capacity);
            vkUnmapMemory(device, transferBuffer.stagingBufferMemory);

            copyBuffer(transferBuffer.stagingBuffer, transferBuffer.buffer, transferBuffer.capacity, 0,0);
        }

		void destroyTransferBuffer(TransferBuffer transferBuffer);

		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &deviceMemory);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkCommandPool createCommandPool();
		VkCommandBuffer beginSingleTimeCommands(VkCommandPool &commandPool);
		void endSingleTimeCommands(VkCommandBuffer &commandBuffer, VkCommandPool &commandPool);
		static std::vector<char> readFile(const std::string& filename);
		void cleanup();
		
			private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		const VulkanInstance &vulkanInstance;
		const DisplayInstance &displayInstance;
		const VkDevice &device;
};

#endif
