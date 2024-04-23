#include "chunk.hpp"
#include "world_builder.hpp"
#include "chunk_manager.hpp"
#include <memory>

const glm::ivec3 Chunk::chunk_size = {16, 64, 16};
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

int Chunk::push_vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) {
    pos += world_offset;
    uv = tex_offset + uv * tex_size;
    vp.push_back(pos.x);
    vp.push_back(pos.y);
    vp.push_back(pos.z);
    vn.push_back(norm.x);
    vn.push_back(norm.y);
    vn.push_back(norm.z);
    vuv.push_back(uv.x);
    vuv.push_back(uv.y);

    return vert_index++;
}

void Chunk::push_face(DIR dir, int texIndex) {
    tex_offset.x = (float)(texIndex) / tex_num_x;
    tex_offset.y = 0;
    tex_size.x = 1.0 / tex_num_x;
    tex_size.y = 1.0;

    switch (dir) {
        case DIR::UP: {
            push_vertex({1, 1, 0}, {0, 1, 0}, {1, 0});
            push_vertex({0, 1, 0}, {0, 1, 0}, {0, 0});
            push_vertex({1, 1, 1}, {0, 1, 0}, {1, 1});
            push_vertex({1, 1, 1}, {0, 1, 0}, {1, 1});
            push_vertex({0, 1, 0}, {0, 1, 0}, {0, 0});
            push_vertex({0, 1, 1}, {0, 1, 0}, {0, 1});
        } break;
        case DIR::DOWN: {
            push_vertex({0, 0, 0}, {0, -1, 0}, {0, 0});
            push_vertex({1, 0, 0}, {0, -1, 0}, {1, 0});
            push_vertex({1, 0, 1}, {0, -1, 0}, {1, 1});
            push_vertex({0, 0, 0}, {0, -1, 0}, {0, 0});
            push_vertex({1, 0, 1}, {0, -1, 0}, {1, 1});
            push_vertex({0, 0, 1}, {0, -1, 0}, {0, 1});
        } break;
        case DIR::FRONT: {
            push_vertex({0, 0, 1}, {0, 0, 1}, {0, 1});
            push_vertex({1, 0, 1}, {0, 0, 1}, {1, 1});
            push_vertex({1, 1, 1}, {0, 0, 1}, {1, 0});
            push_vertex({0, 0, 1}, {0, 0, 1}, {0, 1});
            push_vertex({1, 1, 1}, {0, 0, 1}, {1, 0});
            push_vertex({0, 1, 1}, {0, 0, 1}, {0, 0});
        } break;
        case DIR::BACK: {
            push_vertex({1, 0, 0}, {0, 0, -1}, {1, 1});
            push_vertex({0, 0, 0}, {0, 0, -1}, {0, 1});
            push_vertex({1, 1, 0}, {0, 0, -1}, {1, 0});
            push_vertex({1, 1, 0}, {0, 0, -1}, {1, 0});
            push_vertex({0, 0, 0}, {0, 0, -1}, {0, 1});
            push_vertex({0, 1, 0}, {0, 0, -1}, {0, 0});
        } break;
        case DIR::RIGHT: {
            push_vertex({0, 1, 0}, {-1, 0, 0}, {0, 0});
            push_vertex({0, 0, 0}, {-1, 0, 0}, {0, 1});
            push_vertex({0, 1, 1}, {-1, 0, 0}, {1, 0});
            push_vertex({0, 1, 1}, {-1, 0, 0}, {1, 0});
            push_vertex({0, 0, 0}, {-1, 0, 0}, {0, 1});
            push_vertex({0, 0, 1}, {-1, 0, 0}, {1, 1});
        } break;
        case DIR::LEFT: {
            push_vertex({1, 0, 0}, {1, 0, 0}, {0, 1});
            push_vertex({1, 1, 0}, {1, 0, 0}, {0, 0});
            push_vertex({1, 1, 1}, {1, 0, 0}, {1, 0});
            push_vertex({1, 0, 0}, {1, 0, 0}, {0, 1});
            push_vertex({1, 1, 1}, {1, 0, 0}, {1, 0});
            push_vertex({1, 0, 1}, {1, 0, 0}, {1, 1});
        } break;
    }
}

void Chunk::build_mesh() {
    vert_index = 0;

    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                world_offset = {x, y, z};
                if (current_block = getBlock({x, y, z})) {
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