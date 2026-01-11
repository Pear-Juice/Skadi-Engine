#include "Phys.hpp"

Phys::Phys() {
  std::cout << "Begin phyics" << std::endl;

  btVector3 gravity(0, -9.8, 0);

    std::cout << "Gravity vector initialized: ("
              << gravity.getX() << ", "
              << gravity.getY() << ", "
              << gravity.getZ() << ")" << std::endl;

    std::cout << "Bullet Physics linked successfully!" << std::endl;
}

Phys::~Phys() {
}
