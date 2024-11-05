#ifndef MESH
#define MESH

#include <cstdint>
#include <vector>

#include "Texture.hpp"
#include "../Graphics/Vertex.hpp"
#include "Dependancies/uuid.h"

struct Mesh {
	uuids::uuid id;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	uuids::uuid materialID;
};

#endif
