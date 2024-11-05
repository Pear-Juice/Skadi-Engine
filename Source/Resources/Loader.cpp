#include "Loader.hpp"
#include "Source/Graphics/Vertex.hpp"
#include <assimp/mesh.h>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <string>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../../Dependancies/stb_image.h"

#define STB_IMAGE_WRITE_IMPLIMENTATION
#include "Mesh.hpp"
#include "Model.hpp"
#include "../../Dependancies/stb_image_write.h"

#include "../../Dependancies/tiny_gltf.h"
#include "Source/IdGen.hpp"

void Loader::processAiNode(const aiScene *scene, aiNode *node, std::vector<Model> &models) {
	std::vector<Texture> textures;
	std::vector<Mesh> meshes;
	Material material{};
	material.id = IDGen::genID();

	for (uint32_t i = 0; i < node->mNumMeshes; i++) {
		Mesh mesh{};
		mesh.id = IDGen::genID();

		aiMesh *assimpMesh = scene->mMeshes[node->mMeshes[i]];

		for (uint32_t j = 0; j < assimpMesh->mNumVertices; j++) {
			Vertex vertex;
			vertex.pos.x = assimpMesh[i].mVertices[j].x;
			vertex.pos.y = assimpMesh[i].mVertices[j].y;
			vertex.pos.z = assimpMesh[i].mVertices[j].z;

			vertex.texCoord.x = assimpMesh->mTextureCoords[0][j].x;
			vertex.texCoord.y = assimpMesh->mTextureCoords[0][j].y;

			mesh.vertices.push_back(vertex);
		}

		for (uint32_t j = 0; j < assimpMesh->mNumFaces; j++) {
			const aiFace& face = assimpMesh->mFaces[j];
			for (uint32_t k = 0; k < face.mNumIndices; k++) {
				mesh.indices.push_back(face.mIndices[k]);
			}
		}

		if (assimpMesh->mMaterialIndex >= 0) {
			aiMaterial *aiMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
			aiString texturePath;


			if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
				const aiTexture *aiTex = scene->GetEmbeddedTexture(texturePath.C_Str());

				Texture texture = loadTexture(aiTex);
				textures.push_back(texture);
				mesh.materialID = material.id;
			}
		}


		meshes.push_back(mesh);
	}

	if (!meshes.empty()) {
		if (!textures.empty()) {
			material.textures = textures;
		}

		Model model;
		model.id = IDGen::genID();
		model.meshes = meshes;
		model.name = node->mName.C_Str();
		model.material = material;
		model.transform = Assimp2Glm(node->mTransformation);

		models.push_back(model);
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		processAiNode(scene, node->mChildren[i], models);
	}
}

std::vector<Model> Loader::loadModels(std::filesystem::path path) {
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	std::cout << "At " << path << "\n";

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "SKADI: Assimp failed to load model into scene object\n";
		return {};
	}

	std::vector<Model> models;
	processAiNode(scene, scene->mRootNode, models);

	return models;
}

Texture Loader::loadTexture(std::filesystem::path filePath) {
	Texture texture{};
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image");
	}

	std::cout << "Loaded texture: " << filePath.c_str() << '\n';

	texture.pixels = pixels;
	texture.width = texWidth;
	texture.height = texHeight;
	texture.channels = texChannels;;
	texture.byteSize = texWidth * texHeight * 4;

	texture.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture.width, texture.height))) + 1);
	texture.id = IDGen::genID();

	return texture;
}


Texture Loader::loadTexture(const aiTexture *aiTexture) {
	Texture texture{};
	uint32_t dataSize;

	if (aiTexture->mHeight == 0) {
		dataSize = aiTexture->mWidth * 4;
	}
	else {
		dataSize = aiTexture->mWidth * aiTexture->mHeight * 4;
	}

	int width;
	int height;
	int numChannels;
	stbi_uc *data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(aiTexture->pcData), dataSize, &width, &height, &numChannels, STBI_rgb_alpha);

	if (!data) {
		throw std::runtime_error("Failed to load assimp texture image");
	}

	texture.pixels = data;
	texture.width = width;
	texture.height = height;
	texture.channels = 4;
	texture.byteSize = width * height * 4;

	texture.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture.width, texture.height))) + 1);
	texture.id = IDGen::genID();

	return texture;
}

