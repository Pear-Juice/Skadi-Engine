#ifndef VULKMESH_HPP
#define VULKMESH_HPP

#include "DynamicBuffer.hpp"
#include "VulkTexture.hpp"

struct VulkMesh {
	Mesh mesh;
	TransferBuffer vertexBuffer;
	TransferBuffer indexBuffer;
	glm::mat4 transform;

	std::vector<VkDescriptorPool> textureDescriptorPools;
	std::vector<VkDescriptorSet> textureDescriptors;
};

#endif //VULKMESH_HPP
