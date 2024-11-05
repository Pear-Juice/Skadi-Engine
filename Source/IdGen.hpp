//
// Created by blankitte on 10/21/24.
//

#ifndef IDGEN_HPP
#define IDGEN_HPP
#include <algorithm>
#include <mutex>
#include "Dependancies/uuid.h"


class IDGen {
public:
    std::mutex countMutex;


    static uuids::uuid genID() {

        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        uuids::uuid_random_generator gen{generator};

        return gen();
    }
};



#endif //IDGEN_HPP
