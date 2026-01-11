#ifndef PHYS_HPP
#define PHYS_HPP

#include <iostream>
#include <btBulletDynamicsCommon.h>

class Phys {
  public:
    Phys();
    
    void step();

    ~Phys();
};

#endif
