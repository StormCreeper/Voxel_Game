#include "world_builder.hpp"

uint8_t WorldBuilder::generation_function(glm::ivec3 world_pos) {
    float freq = 0.1;
    float amp = 10.;
    float height_off = 0;
    float offset = 0;
    for (int i = 0; i < 8; i++) {
        height_off += sin((cos(offset) * world_pos.x + sin(offset) * world_pos.z) * freq) * amp;
        offset += 9.10929;
        freq *= 2;
        amp /= 2;
    }
    int height = 50 + height_off * 0.3;
    if (world_pos.y < height - 5) return 1;
    if (world_pos.y < height - 1) return 2;
    if (world_pos.y < height) return 3;
    return 0;
}