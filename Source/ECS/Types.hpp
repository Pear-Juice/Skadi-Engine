#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include "DataStorage/SparseSet.hpp"

using Entity = uint32_t;
constexpr uint32_t COMPONENT_COUNT = 32;
using Signature = std::bitset<COMPONENT_COUNT>;
using ComponentType = uint8_t;

#endif //TYPES_HPP
