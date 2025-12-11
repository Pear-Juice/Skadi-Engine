//
// Created by blankitte on 10/12/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "Texture.hpp"
#include "Dependencies/uuid.h"

struct Material {
    uuids::uuid id;
    std::unordered_map<std::string, Texture> textures;
};

#endif //MATERIAL_HPP
