#ifndef WORLD_BUILDER_HPP
#define WORLD_BUILDER_HPP

#include "gl_includes.hpp"

class WorldBuilder {
   public:
    static uint8_t generation_function(glm::ivec3 world_pos);

   private:
};

#endif  // WORLD_BUILDER_HPP