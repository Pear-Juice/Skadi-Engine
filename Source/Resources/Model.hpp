#ifndef MODEL
#define MODEL

#include <cstdint>
#include <vector>

#include "Material.hpp"
#include "Mesh.hpp"
#include "Dependencies/uuid.h"

struct Model {
	uuids::uuid id;
	std::string name;
	std::vector<Mesh> meshes;
	glm::mat4 transform;

	Material material;
};

#endif
