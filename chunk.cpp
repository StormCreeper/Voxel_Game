#include "chunk.hpp"
#include "world_builder.hpp"
#include "chunk_manager.hpp"

const glm::ivec3 Chunk::chunk_size = {16, 64, 16};
std::shared_ptr<Texture> Chunk::chunk_texture{};

void Chunk::voxel_map_from_noise() {
    if (!allocated)
        allocate();

    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                glm::ivec3 world_pos{
                    x + chunk_size.x * pos.x,
                    y,
                    z + chunk_size.z * pos.y};
                voxelMap[index(x, y, z)] = WorldBuilder::generation_function(world_pos);
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
    vp.clear();
    vn.clear();
    vuv.clear();

    vert_index = 0;

    int bottom_tex = 0;
    int side_tex = 1;
    int top_tex = 2;

    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                world_offset = {x, y, z};
                if (current_block = getBlock(x, y, z)) {
                    if (current_block == 1) {
                        bottom_tex = 3;
                        side_tex = 3;
                        top_tex = 3;
                    } else if (current_block == 2) {
                        bottom_tex = 0;
                        side_tex = 0;
                        top_tex = 0;
                    } else {
                        bottom_tex = 0;
                        side_tex = 1;
                        top_tex = 2;
                    }

                    if (!getBlock(x, y, z + 1)) push_face(DIR::FRONT, side_tex);
                    if (!getBlock(x, y, z - 1)) push_face(DIR::BACK, side_tex);
                    if (!getBlock(x, y - 1, z)) push_face(DIR::DOWN, bottom_tex);
                    if (!getBlock(x, y + 1, z)) push_face(DIR::UP, top_tex);
                    if (!getBlock(x + 1, y, z)) push_face(DIR::LEFT, side_tex);
                    if (!getBlock(x - 1, y, z)) push_face(DIR::RIGHT, side_tex);
                }
            }
        }
    }

    mesh->initGPUGeometry(vp, vn, vuv);
}

uint8_t Chunk::getBlock(int x, int y, int z, bool rec) {
    if (!allocated) return 0;
    if (x < 0 || y < 0 || z < 0 ||
        x >= chunk_size.x || y >= chunk_size.y || z >= chunk_size.z) {
        if (rec)
            return chunk_manager->getBlock(glm::ivec3(
                x + pos.x * chunk_size.x,
                y,
                z + pos.y * chunk_size.z));
        return 0;
    }

    return voxelMap[index(x, y, z)];
}