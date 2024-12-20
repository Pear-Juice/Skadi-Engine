#ifndef VULKMESH_HPP
#define VULKMESH_HPP

#include "TransferBuffer.hpp"
#include "VulkTexture.hpp"

struct VulkMesh {
	Mesh mesh;
	TransferBuffer vertexBuffer;
	TransferBuffer indexBuffer;

	std::vector<VkDescriptorPool> textureDescriptorPools;
	std::vector<VkDescriptorSet> textureDescriptors;
};

#endif //VULKMESH_HPP
