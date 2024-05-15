#include "world_builder.hpp"

SimplexNoise WorldBuilder::sn{0.005f, 1.0f};

uint8_t WorldBuilder::generation_function(glm::ivec3 world_pos) {
    float mountain_val = abs(sn.fractal(8, world_pos.x, world_pos.z)) * 2.0f - 1.0f;
    float plain_val = sn.fractal(4, world_pos.x * 0.4, world_pos.z * 0.4);
    float sea_val = sn.fractal(8, world_pos.x * 0.2, world_pos.z * 0.2);

    float L = 1;
    float k = 18;
    float x0 = 0;
    float base_h = 0.2;

    sea_val = L / (1 + exp(-k * (sea_val - x0))) * (1 - base_h) + base_h;

    float lerp = sn.fractal(8, world_pos.x * 0.3, world_pos.z * 0.3) * 0.5f + 0.5f;

    float val = mountain_val * lerp + plain_val * (1 - lerp);
    val = pow(val * 0.5f + 0.5f, 2.0f) * 2.0f - 1.0f;

    int height = (34 + val * 30) * sea_val;
    if (world_pos.y < height - 5) return 1;
    if (world_pos.y < height - 1) return 2;
    if (world_pos.y < height) {
        if (mountain_val < -0.9f && lerp > 0.5f) return 9;
        return 3;
    }
    if (world_pos.y < 5) return 9;
    return 0;
}