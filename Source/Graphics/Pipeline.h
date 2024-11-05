#ifndef PIPELINE
#define PIPELINE

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class Pipeline {
	void addShader(std::string full_path, std::string type);
	VkPipeline generateVkPipeline();

	VkDevice logicalDevice;
	std::vector<VkShaderModule> shaders;
	VkPipelineShaderStageCreateInfo createVertShader(VkShaderModule shader);
	VkPipelineShaderStageCreateInfo createFragShader(VkShaderModule shader);

};

#endif
