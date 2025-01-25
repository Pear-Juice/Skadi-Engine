#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include <cstdint>
#include <bitset>

using Entity = uint32_t;
constexpr uint32_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;
using ComponentType = uint8_t;

#endif //DEFINITIONS_HPP
