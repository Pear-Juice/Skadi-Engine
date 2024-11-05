#ifndef VULKTEXTURE_HPP
#define VULKTEXTURE_HPP

struct VulkTexture {
	Texture texture;

	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler imageSampler;
};

#endif //VULKTEXTURE_HPP
