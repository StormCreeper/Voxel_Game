#ifndef WORLD_BUILDER_HPP
#define WORLD_BUILDER_HPP

#include "utils/gl_includes.hpp"
#include "SimplexNoise.h"

class WorldBuilder {
   public:
    static uint8_t generation_function(glm::ivec3 world_pos);

   private:
    static SimplexNoise sn;
};

#endif  // WORLD_BUILDER_HPP