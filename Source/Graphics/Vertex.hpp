#ifndef VERTEX
#define VERTEX

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>
#include <array>
#include <string>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color{1,1,1};
	glm::vec2 texCoord;

	//Bindings are for the struct and describe how it should be passed into the vertex shader
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	
	//Attribute descriptions describe for each property what its format should be and where it will be in memory
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	//define comparison between two vertices
	bool operator==(const Vertex &other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

//custom hash function for Vertex (uses hashes of vec 2 and 3)
template <>
struct std::hash<Vertex>
{
  std::size_t operator()(const Vertex& vertex) const
  {
	using std::size_t;
	using std::hash;
	using std::string;

	// Compute individual hash values for first,
	// second and third and combine them using XOR
	// and bit shifting:

	return ((hash<glm::vec3>()(vertex.pos)
			 ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
			 ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};

#endif
