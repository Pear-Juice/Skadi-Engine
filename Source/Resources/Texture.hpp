#ifndef TEXTURE
#define TEXTURE

#include <cstdint>
#include <string>
#include <Dependancies/stb_image.h>
#include <Dependancies/uuid.h>

enum TextureType {

};

struct Texture {
	uuids::uuid id;
	TextureType type;

	stbi_uc* pixels;
	uint32_t width;
	uint32_t height;
	uint32_t channels;
	uint32_t byteSize;

	uint32_t mipLevels;
};

#endif
