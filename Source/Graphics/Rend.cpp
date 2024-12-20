#include "Vertex.hpp"
#include <algorithm>
#include <array>
#include <bits/fs_fwd.h>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <string>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <limits>
#include <iostream>
#include <sys/types.h>
#include <unordered_map>
#include <cstring>
#include <chrono>

#include "Rend.hpp"

#include <Source/Resources/Loader.hpp>

#include "DynamicBuffer.hpp"
#include "DisplayInstance.hpp"
#include "VulkanInstance.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

VkDevice device;
VkPhysicalDevice physicalDevice;
GLFWwindow *window;

Rend::Rend() {
	displayInstance = new DisplayInstance();
	vInstance = new VulkanInstance();

	displayInstance->initDisplay(vInstance->instance, WIDTH, HEIGHT, "Skadi Engine");
	vInstance->setupDevices(*displayInstance);

	resourceManager = new ResourceManager(*vInstance, *displayInstance);

	camera = Camera(glm::mat4(1), 45, WIDTH, HEIGHT);

	device = vInstance->device;
	physicalDevice = displayInstance->physicalDevice;
	window = displayInstance->window;
}

void Rend::beginLoop() {
	std::cout << "Run" << '\n';
	
	renderThread = std::thread(&Rend::mainLoop, this);
}

void Rend::mainLoop() {
	while (!glfwWindowShouldClose(displayInstance->window)) {
		auto start = std::chrono::steady_clock::now();

		glfwPollEvents();
		drawFrame();

		auto end = std::chrono::steady_clock::now();
		auto frame_elapsed_millis = std::chrono::duration_cast<std::chrono::duration<float,std::milli>>(end-start).count();

		if (frame_elapsed_millis <= maxFrameTimeMilli) {
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(maxFrameTimeMilli - frame_elapsed_millis)));
		}

		//Uncomment to print render time
		// auto total_end = std::chrono::steady_clock::now();
		// auto total_elapsed_millis = std::chrono::duration_cast<std::chrono::duration<float,std::milli>>(total_end-start).count();

		// std::cout << total_elapsed_millis << "\n";
	}

	vkDeviceWaitIdle(device);
	cleanup();
}

void Rend::setMaxFPS(int fps) {
	maxFrameTimeMilli = 1.0f / static_cast<float>(fps) * 1000.0f;
}


void Rend::initVulkan() {
	createSwapChain(); 
	createImageViews();
	createRenderPass();

	uboDescriptorSetLayout = resourceManager->createDescriptorSetLayout(1,0,0);
	samplerDescriptorSetLayout = resourceManager->createDescriptorSetLayout(0,1,0);

	VkShaderModule vertShader = resourceManager->createShaderModule(resourceManager->readFile("/home/blankitte/Documents/Engines/SkadiEngine/Shaders/vert.spv"));
	VkShaderModule fragShader = resourceManager->createShaderModule(resourceManager->readFile("/home/blankitte/Documents/Engines/SkadiEngine/Shaders/frag.spv"));
	VkPushConstantRange transformRange{};
	transformRange.offset = 0;
	transformRange.size = 64;
	transformRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	graphicsPipeline = resourceManager->createGraphicsPipeline(vertShader, fragShader, swapChainExtent, renderPass, displayInstance->msaaSamples, {uboDescriptorSetLayout, samplerDescriptorSetLayout},{transformRange});
	vkDestroyShaderModule(device, vertShader, nullptr);
	vkDestroyShaderModule(device, fragShader, nullptr);

	commandPool = resourceManager->createCommandPool();
	createColorResources();
	createDepthResources();
	createFramebuffers();

	createUniformBuffers();
	createCommandBuffers();
	createSyncObjects();

	uboDescriptorPool = resourceManager->createDescriptorPool(MAX_FRAMES_IN_FLIGHT, 1, 0);
	uboDescriptorSets = resourceManager->createUBODescriptorSets(uboDescriptorPool, uboDescriptorSetLayout, uniformBuffers, MAX_FRAMES_IN_FLIGHT);

	std::cout << "Initialized Vulkan\n";
}

void Rend::renderMesh(Mesh mesh) {
	meshQueue.push(mesh);
}

void Rend::updateMeshTransform(uuids::uuid uuid, glm::mat4 transform) {
	if (vulkMeshes.contains(uuid))
		vulkMeshes[uuid].mesh.transform = transform;
	else
		std::cout << "REND: Mesh ID: " << uuid << " is not a model ID\n";
}

void Rend::updateMesh(uuids::uuid uuid, Mesh mesh) {
	
}

void Rend::eraseMesh(uuids::uuid uuid) {

}

//Change to material that has list of textures
void Rend::registerMaterial(Material &material) {
	materialSubmitMutex.lock();

	VulkMaterial vulkMaterial{};
	vulkMaterial.pool = resourceManager->createDescriptorPool(MAX_FRAMES_IN_FLIGHT, 0, 1);
	vulkMaterial.layout = resourceManager->createDescriptorSetLayout(0,1,0);

	for (auto [id, texture] : material.textures) {
		vulkMaterial.textures.push_back(createVulkTexture(texture));
	}

	if (vulkMaterials.find(material.id) != vulkMaterials.end()) {
		std::cout << "Failed to register material. ID: " << material.id << " is already registered.\n";
		return;
	}

	vulkMaterial.sets = resourceManager->createImageDescriptorSets(vulkMaterial.pool, vulkMaterial.layout, vulkMaterial.textures, MAX_FRAMES_IN_FLIGHT);
	vulkMaterials[material.id] = vulkMaterial;

	materialSubmitMutex.unlock();
}

void Rend::processMeshQueue() {
	meshQueueSubmitMutex.lock();

	if (!meshQueue.empty())
		std::cout << "REND: Queuing " << meshQueue.size() << " meshes\n";
	while (!meshQueue.empty()) {
		Mesh mesh = meshQueue.front();

		TransferBuffer vertexBuffer = resourceManager->createTransferBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh.vertices.size() * sizeof(Vertex));
		TransferBuffer indexBuffer = resourceManager->createTransferBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mesh.indices.size() * sizeof(uint32_t));

		resourceManager->transferBufferWrite<Vertex>(vertexBuffer, mesh.vertices);
		resourceManager->transferBufferWrite<uint32_t>(indexBuffer, mesh.indices);

		VulkMesh vulkMesh{};
		vulkMesh.mesh = mesh;
		vulkMesh.vertexBuffer = vertexBuffer;
		vulkMesh.indexBuffer = indexBuffer;

		std::vector<VkDescriptorSet> bindSets;

		try {
			bindSets = vulkMaterials.at(mesh.materialID).sets;
		}
		catch(const std::out_of_range &e)
		{
			std::cout << "Failed to bind texture: " << mesh.materialID << '\n';
		}

		vulkMesh.textureDescriptors = bindSets;

		vulkMeshes[mesh.id] = vulkMesh;

		meshQueue.pop();
	}

	meshQueueSubmitMutex.unlock();
}

///Searches available swap formats for SRGB 32 bit and returns it if it exits
///If not it chooses the first available format
VkSurfaceFormatKHR Rend::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}


VkPresentModeKHR Rend::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Rend::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	VkExtent2D actualExtent = {
		displayInstance->windowWidth,
		displayInstance->windowHeight
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void Rend::createSwapChain() {
	DisplayInstance::SwapChainSupportDetails swapChainSupport = displayInstance->querySwapChainSupport(physicalDevice, displayInstance->surface);
	
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);


	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = displayInstance->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	DisplayInstance::QueueFamilyIndecies indices = displayInstance->queueFamilyIndecies;
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount, nullptr);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Rend::cleanupSwapChain() {
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);	
}

void Rend::recreateSwapChain() {
	//int width = 0, height = 0;
	//glfwGetFramebufferSize(window, &width, &height);
	//std::cout << "Create swap" << "\n";
	while (displayInstance->windowWidth == 0 || displayInstance->windowHeight == 0) {
		if (glfwWindowShouldClose(window)) return;

		//glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createColorResources();
	createDepthResources();
	createFramebuffers();
}

void Rend::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = resourceManager->createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);	
	}
}

void Rend::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());
	
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			colorImageView,
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer");
		}
	}
}

VkFormat Rend::findSupportedImageFormat(const std::vector<VkFormat>&candidates, VkImageTiling tiling, VkFormatFeatureFlags featureFlags) {
	for (VkFormat format : candidates) {
		VkFormatProperties formatProps{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		
		if (tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures &featureFlags) == featureFlags) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & featureFlags) == featureFlags) {
			return format;
		}

	}

	throw std::runtime_error("Failed to find supported image format");
}

VkFormat Rend::findDepthFormat() {
	return findSupportedImageFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}
			,VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);		
}

bool Rend::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Rend::createDepthResources() {
	VkFormat depthFormat = findDepthFormat();

	resourceManager->createImage(swapChainExtent.width, swapChainExtent.height, 1, displayInstance->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = resourceManager->createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Rend::createColorResources() {
	VkFormat colorFormat = swapChainImageFormat;

	resourceManager->createImage(swapChainExtent.width, swapChainExtent.height, 1, displayInstance->msaaSamples, colorFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			colorImage, colorImageMemory);

	colorImageView = resourceManager->createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VulkTexture Rend::createVulkTexture(Texture texture) {
	VulkTexture vulkTexture{};

	updateImage(texture, vulkTexture);
	updateImageView(vulkTexture);
	updateSampler(displayInstance->physicalDeviceProperties, vulkTexture);

	return vulkTexture;
}

void Rend::updateImage(Texture texture, VulkTexture &vulkTexture) {
	vulkTexture.texture = texture;

	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};
	resourceManager->createBuffer(texture.byteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	
	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, texture.byteSize, 0, &data);
	memcpy(data, texture.pixels, texture.byteSize);
	vkUnmapMemory(device, stagingBufferMemory);

	resourceManager->createImage(texture.width, texture.height, texture.mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkTexture.image, vulkTexture.imageMemory);

	//Transition image to be able to be copied to
	resourceManager->transitionImageLayout(vulkTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);

	//copy to staging buffer to image
	resourceManager->copyBufferToImage(stagingBuffer, vulkTexture.image, texture.width, texture.height);

	//transition image to be readable by shader
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
	
	//mipmap generation should not be done at runtime but this does work
	generateMipMaps(vulkTexture.image, VK_FORMAT_R8G8B8A8_SRGB, texture.width,texture.height, texture.mipLevels);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Rend::updateSampler(VkPhysicalDeviceProperties physicalDeviceProperties, VulkTexture &vulkTexture) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; //defines what happens when you sample outside an image
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // REPEAT loops the image, MIRRORED_REPEAT loops mirrored
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;

	samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //if image sampling does not loop what color should it be
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.compareEnable = VK_FALSE; //Used for percentage closer filtering. shadow maps, do not understand
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxLod = static_cast<float>(vulkTexture.texture.mipLevels);

	VkSampler imageSampler{};
	if (vkCreateSampler(device, &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	vulkTexture.imageSampler = imageSampler;
}

void Rend::updateImageView(VulkTexture &vulkTexture) {
	vulkTexture.imageView = resourceManager->createImageView(vulkTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, vulkTexture.texture.mipLevels);
}

void Rend::generateMipMaps(VkImage image, VkFormat imageFormat, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels) {
	//Make sure image can be mipmapped
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("Texture image format does not support linear blitting");
	}
	
	VkCommandPool commandPool = resourceManager->createCommandPool();
	VkCommandBuffer commandBuffer = resourceManager->beginSingleTimeCommands(commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	//mip from current level (starts at 0) to number of mips. I is the level we are mipping to
	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		//create a memory barrier for image write operations
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
				);

		VkImageBlit blit{};
		blit.srcOffsets[0] = {0,0,0};
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = {0,0,0};
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		//blit image to be half the size store it in mip index
		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//transition mip image to be readable by shaders
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

		//scale down current mip size if possible
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	//transfer last image to be readable by shaders
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	resourceManager->endSingleTimeCommands(commandBuffer, commandPool);
}

void Rend::createCommandBuffers() {
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers");
	}
}

void Rend::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		resourceManager->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}



void Rend::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = displayInstance->msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = displayInstance->msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}

void Rend::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
		vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore");
		}
	}
}

///Records GPU commands to a buffer to be submitted to the graphics queue
void Rend::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

	renderPassInfo.renderArea.offset = {0,0};
	renderPassInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{1.0f, 0.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// UniformBufferObject ubo{};
	// //ubo.view = glm::lookAt(glm::vec3(2.0f,2.0f,2.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
	// //ubo.view = glm::rotate(translate(glm::mat4(1), glm::vec3(0,0,-4)), glm::radians(-50.0f), glm::vec3(1,0,0));
	// ubo.view = glm::translate(glm::rotate(glm::mat4(1), glm::radians(30.0f), glm::vec3(1,0,0)), glm::vec3(0,-5,10));
	// ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 100.0f);
	//

	if (displayInstance->windowResized) {
		camera.setViewport(displayInstance->windowWidth, displayInstance->windowHeight);
		std::cout << "resize to: " << displayInstance->windowWidth << ", " << displayInstance->windowHeight << "\n";
	}

	updateUniformBuffer(camera.getUBO());

	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.layout, 0, 1, &uboDescriptorSets[frame], 0, nullptr);

	for (const auto & [ id, vulkMesh ] : vulkMeshes) {
		std::vector sets{uboDescriptorSets[frame],vulkMesh.textureDescriptors[frame]};
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.layout, 0, sets.size(), sets.data(), 0, nullptr);

		VkBuffer vertexBuffers[] = {vulkMesh.vertexBuffer.buffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, vulkMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);


		glm::mat4 transform = vulkMesh.mesh.transform;
		vkCmdPushConstants(commandBuffer, graphicsPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &transform);

		vkCmdDrawIndexed(commandBuffer, vulkMesh.indexBuffer.objectCount, 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer");
	}
}

void Rend::drawFrame() {
	vkWaitForFences(device, 1, &inFlightFences[frame], VK_TRUE, UINT64_MAX);
	
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[frame], VK_NULL_HANDLE, &imageIndex);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to aquire swapchain image");
	}

	vkResetFences(device, 1, &inFlightFences[frame]);

	vkResetCommandBuffer(commandBuffers[frame], 0);
	recordCommandBuffer(commandBuffers[frame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[frame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[frame];

	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[frame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(vInstance->graphicsQueue, 1, &submitInfo, inFlightFences[frame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(vInstance->presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || displayInstance->windowResized) {
		displayInstance->windowResized = false;
		recreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image");
	}

	frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;

	processMeshQueue();
}

void Rend::updateUniformBuffer(UniformBufferObject ubo) {

	memcpy(uniformBuffersMapped[frame], &ubo, sizeof(ubo));
}

void Rend::cleanupVulkMaterial(VulkMaterial material) {
	for (const VulkTexture texture : material.textures) {
		std::cout << "Destroy texture: " << texture.texture.id << "\n";
		vkDestroySampler(device, texture.imageSampler, nullptr);
		vkDestroyImageView(device, texture.imageView, nullptr);
		vkDestroyImage(device, texture.image, nullptr);
		vkFreeMemory(device, texture.imageMemory, nullptr);
		stbi_image_free(texture.texture.pixels);
	}

	vkDestroyDescriptorSetLayout(device, material.layout, nullptr);
	vkDestroyDescriptorPool(device, material.pool, nullptr);
}

void Rend::cleanup() {
	for (const auto & [ id, vulkMesh ] : vulkMeshes) {
		resourceManager->destroyTransferBuffer(vulkMesh.vertexBuffer);
		resourceManager->destroyTransferBuffer(vulkMesh.indexBuffer);
	}

	for (const auto & [id, vulkMaterial] : vulkMaterials) {
		cleanupVulkMaterial(vulkMaterial);
	}
	vkDestroyDescriptorSetLayout(device, samplerDescriptorSetLayout, nullptr);

	cleanupSwapChain();
		
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, uboDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, uboDescriptorSetLayout, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyPipeline(device, graphicsPipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(device, graphicsPipeline.layout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
		
	resourceManager->cleanup();
	delete resourceManager;
	displayInstance->cleanup(vInstance->instance);
	delete displayInstance;
	vInstance->cleanup();
	delete vInstance;

	std::cout << "Finished cleanup\n";
}
