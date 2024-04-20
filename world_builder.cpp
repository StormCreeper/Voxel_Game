#include "world_builder.hpp"

uint8_t WorldBuilder::generation_function(glm::ivec3 world_pos) {
    int height = 55 + 9 * (sin((world_pos.x + world_pos.z) * 0.2) * 0.5 + 0.5);
    if (world_pos.y < height - 5) return 1;
    if (world_pos.y < height - 1) return 2;
    if (world_pos.y < height) return 3;
    return 0;
}