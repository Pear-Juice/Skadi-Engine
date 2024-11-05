//
// Created by blankitte on 10/12/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "Texture.hpp"
#include "Dependancies/uuid.h"

struct Material {
    uuids::uuid id;
    std::vector<uuids::uuid> textures;
};

#endif //MATERIAL_HPP
