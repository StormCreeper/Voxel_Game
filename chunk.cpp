#include "chunk.hpp"
#include "world_builder.hpp"
#include "chunk_manager.hpp"
#include <memory>

std::shared_ptr<Texture> Chunk::chunk_texture{};

void Chunk::voxel_map_from_noise() {
    if (!allocated)
        allocate();

    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                voxelMap[index({x, y, z})] =
                    WorldBuilder::generation_function({x + chunk_size.x * pos.x,
                                                       y,
                                                       z + chunk_size.z * pos.y});
            }
        }
    }
}

void Chunk::push_vertex(glm::ivec3 pos, glm::vec2 uv) {
    pos += world_offset;
    uv = tex_offset + uv * tex_size;
    GLuint ipos = pos.x + pos.z * (Chunk::chunk_size.x + 1) + pos.y * (Chunk::chunk_size.x + 1) * (Chunk::chunk_size.z + 1);
    vp.push_back(ipos);
    vn.push_back(light_level);
    vuv.push_back(uv.x);
    vuv.push_back(uv.y);
}

void Chunk::push_face(DIR dir, int texIndex) {
    tex_size.x = 1.0 / tex_num_x;
    tex_size.y = 1.0 / tex_num_y;
    tex_offset.x = (texIndex % tex_num_x) * tex_size.x;
    tex_offset.y = (texIndex / tex_num_x) * tex_size.y;

    light_level = BlockPalette::face_light[dir];

    uint8_t light_value = get_light_value(world_offset);
    int block_light = light_value & 0b00001111;
    int sky_light = (light_value & 0b11110000) >> 4;

    light_level *= (float)std::max(block_light, sky_light) / 15.0f;

    switch (dir) {
        case DIR::UP: {
            push_vertex({1, 1, 0}, {1, 0});
            push_vertex({0, 1, 0}, {0, 0});
            push_vertex({1, 1, 1}, {1, 1});
            push_vertex({1, 1, 1}, {1, 1});
            push_vertex({0, 1, 0}, {0, 0});
            push_vertex({0, 1, 1}, {0, 1});
        } break;
        case DIR::DOWN: {
            push_vertex({0, 0, 0}, {0, 0});
            push_vertex({1, 0, 0}, {1, 0});
            push_vertex({1, 0, 1}, {1, 1});
            push_vertex({0, 0, 0}, {0, 0});
            push_vertex({1, 0, 1}, {1, 1});
            push_vertex({0, 0, 1}, {0, 1});
        } break;
        case DIR::FRONT: {
            push_vertex({0, 0, 1}, {0, 1});
            push_vertex({1, 0, 1}, {1, 1});
            push_vertex({1, 1, 1}, {1, 0});
            push_vertex({0, 0, 1}, {0, 1});
            push_vertex({1, 1, 1}, {1, 0});
            push_vertex({0, 1, 1}, {0, 0});
        } break;
        case DIR::BACK: {
            push_vertex({1, 0, 0}, {1, 1});
            push_vertex({0, 0, 0}, {0, 1});
            push_vertex({1, 1, 0}, {1, 0});
            push_vertex({1, 1, 0}, {1, 0});
            push_vertex({0, 0, 0}, {0, 1});
            push_vertex({0, 1, 0}, {0, 0});
        } break;
        case DIR::RIGHT: {
            push_vertex({0, 1, 0}, {0, 0});
            push_vertex({0, 0, 0}, {0, 1});
            push_vertex({0, 1, 1}, {1, 0});
            push_vertex({0, 1, 1}, {1, 0});
            push_vertex({0, 0, 0}, {0, 1});
            push_vertex({0, 0, 1}, {1, 1});
        } break;
        case DIR::LEFT: {
            push_vertex({1, 0, 0}, {0, 1});
            push_vertex({1, 1, 0}, {0, 0});
            push_vertex({1, 1, 1}, {1, 0});
            push_vertex({1, 0, 0}, {0, 1});
            push_vertex({1, 1, 1}, {1, 0});
            push_vertex({1, 0, 1}, {1, 1});
        } break;
    }
}

void Chunk::build_mesh() {
    generateLightMap();
    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                world_offset = {x, y, z};
                if (uint8_t current_block = getBlock({x, y, z})) {
                    BlockDesc bd = BlockPalette::get_block_desc(current_block);

                    if (!getBlock({x, y + 1, z})) push_face(DIR::UP, bd.face_indices[DIR::UP]);
                    if (!getBlock({x, y - 1, z})) push_face(DIR::DOWN, bd.face_indices[DIR::DOWN]);
                    if (!getBlock({x + 1, y, z})) push_face(DIR::LEFT, bd.face_indices[DIR::LEFT]);
                    if (!getBlock({x - 1, y, z})) push_face(DIR::RIGHT, bd.face_indices[DIR::RIGHT]);
                    if (!getBlock({x, y, z + 1})) push_face(DIR::FRONT, bd.face_indices[DIR::FRONT]);
                    if (!getBlock({x, y, z - 1})) push_face(DIR::BACK, bd.face_indices[DIR::BACK]);
                }
            }
        }
    }

    mesh->initGPUGeometry(vp, vn, vuv);

    vp.clear();
    vn.clear();
    vuv.clear();
}

void Chunk::generateLightMap() {
    std::fill(lightMap, lightMap + num_blocks, 0b11111111);
}

uint8_t Chunk::getBlock(glm::ivec3 block_pos, bool rec) {
    if (!allocated) return 0;
    if (block_pos.x < 0 || block_pos.y < 0 || block_pos.z < 0 ||
        block_pos.x >= chunk_size.x || block_pos.y >= chunk_size.y || block_pos.z >= chunk_size.z) {
        if (rec)
            return chunk_manager->getBlock({block_pos.x + pos.x * chunk_size.x,
                                            block_pos.y,
                                            block_pos.z + pos.y * chunk_size.z});
        return 0;
    }

    return voxelMap[index(block_pos)];
}

void Chunk::setBlock(glm::ivec3 block_pos, uint8_t block) {
    if (!allocated) return;
    if (block_pos.x < 0 || block_pos.y < 0 || block_pos.z < 0 ||
        block_pos.x >= chunk_size.x || block_pos.y >= chunk_size.y || block_pos.z >= chunk_size.z) {
        return;
    }

    hasBeenModified = true;

    voxelMap[index(block_pos)] = block;
}

inline uint8_t Chunk::get_light_value(glm::ivec3 block_pos) {
    if (!allocated) return 0;
    if (block_pos.x < 0 || block_pos.y < 0 || block_pos.z < 0 ||
        block_pos.x >= chunk_size.x || block_pos.y >= chunk_size.y || block_pos.z >= chunk_size.z) {
        return 0;
    }
    return lightMap[index(block_pos)];
}