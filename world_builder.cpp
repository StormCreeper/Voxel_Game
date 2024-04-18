#include "world_builder.hpp"

int WorldBuilder::generation_function(glm::ivec3 world_pos) {
    return 55 + 9 * (sin((world_pos.x + world_pos.z) * 0.2) * 0.5 + 0.5) > world_pos.y;
}