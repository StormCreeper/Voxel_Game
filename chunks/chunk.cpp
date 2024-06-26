#include "chunk.hpp"
#include "../world_builder.hpp"
#include "chunk_manager.hpp"
#include <memory>

std::shared_ptr<Texture> Chunk::chunk_texture{};

Chunk::Chunk(glm::ivec2 pos, ChunkManager *chunk_manager) {
    chunk_mesh.mesh = std::make_shared<Mesh>();
    this->chunk_manager = chunk_manager;

    chunk_mesh.mesh->genBuffers();

    allocate();
}

void Chunk::init(glm::ivec2 pos) {
    this->pos = pos;
    chunk_mesh.modelMatrix = glm::translate(chunk_mesh.modelMatrix, glm::vec3(pos.x * chunk_size.x, 0, pos.y * chunk_size.z));
}

void Chunk::allocate() {
    voxelMap = (uint8_t *)realloc(voxelMap, num_blocks * sizeof(uint8_t));
    lightMap = (uint8_t *)realloc(lightMap, num_blocks * sizeof(uint8_t));
    memset(lightMap, 0b11111111, num_blocks * sizeof(uint8_t));

    if (!voxelMap || !lightMap) {
        std::cout << "NOOOOOOO no room left :( youre computer is ded :(\n";
        exit(-1);
    }

    state = BlockArrayInitialized;
}

void Chunk::free_mem() {
    if (voxelMap)
        free(voxelMap);
    if (lightMap)
        free(lightMap);
    state = EmptyChunk;
}

void Chunk::voxel_map_from_noise() {
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
    chunk_mesh.vp.push_back(ipos);
    chunk_mesh.vn.push_back(light_level);
    chunk_mesh.vuv.push_back(uv.x);
    chunk_mesh.vuv.push_back(uv.y);
}

void Chunk::push_face(DIR dir, int texIndex) {
    tex_size.x = 1.0 / tex_num_x;
    tex_size.y = 1.0 / tex_num_y;
    tex_offset.x = (texIndex % tex_num_x) * tex_size.x;
    tex_offset.y = (texIndex / tex_num_x) * tex_size.y;

    light_level = BlockPalette::face_light[dir];

    uint8_t light_value = get_light_value(world_offset + BlockPalette::Normal[dir], true);
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
    if (state < LightMapGenerated) {
        std::cout << "Error: tried to build mesh based on incomplete data (lightmap)\n";
        return;
    }
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

    state = MeshBuilt;
}

void Chunk::send_mesh_to_gpu() {
    if (state == MeshBuilt) {
        chunk_mesh.mesh->initGPUGeometry(chunk_mesh.vp, chunk_mesh.vn, chunk_mesh.vuv);

        chunk_mesh.vp.clear();
        chunk_mesh.vn.clear();
        chunk_mesh.vuv.clear();

        state = Ready;
    } else {
        std::cout << "Error : tried to send unvalid mesh data to CPU\n";
    }
}

void Chunk::generateLightMap() {
    state = LightMapGenerated;
    return;
    std::fill(lightMap, lightMap + num_blocks, 0b00000001);
    for (int x = 0; x < Chunk::chunk_size.x; x++) {
        for (int z = 0; z < Chunk::chunk_size.z; z++) {
            int l = 15;
            for (int y = Chunk::chunk_size.y - 1; y >= 0 && l > 0; y--) {
                if (getBlock({x, y, z}))
                    break;
                else
                    lightMap[index({x, y, z})] |= l << 4;
            }
        }
    }
    for (int x = 0; x < Chunk::chunk_size.x; x++) {
        for (int z = 0; z < Chunk::chunk_size.z; z++) {
            for (int y = Chunk::chunk_size.y - 1; y >= 0; y--) {
                uint8_t lv = (get_light_value({x, y, z}) & 0b11110000) >> 4;
                if (lv)
                    floodFill({x, y, z}, lv, true, true);
            }
        }
    }

    state = LightMapGenerated;
}

void Chunk::floodFill(glm::ivec3 block_pos, uint8_t value, bool sky, bool first) {
    if (off_bounds(block_pos)) {
        return;
    }
    if (getBlock(block_pos)) return;

    uint8_t lv = (get_light_value(block_pos) & 0b11110000) >> 4;
    if ((value > lv || first) && value > 0) {
        set_sky_light(block_pos, value);
        glm::ivec3 p{};

        for (int i = 0; i < 6; i++) {
            floodFill(block_pos + BlockPalette::Normal[i], value - 1, sky, false);
        }
    }
}

uint8_t Chunk::getBlock(glm::ivec3 block_pos, bool rec) {
    if (state < BlockArrayInitialized) return 0;
    if (off_bounds(block_pos)) {
        if (rec)
            return chunk_manager->getBlock({block_pos.x + pos.x * chunk_size.x,
                                            block_pos.y,
                                            block_pos.z + pos.y * chunk_size.z});
        return 0;
    }

    return voxelMap[index(block_pos)];
}

void Chunk::setBlock(glm::ivec3 block_pos, uint8_t block) {
    if (state < BlockArrayInitialized) return;
    if (off_bounds(block_pos)) return;

    voxelMap[index(block_pos)] = block;

    hasBeenModified = true;
    state = BlockArrayInitialized;
}

uint8_t Chunk::get_light_value(glm::ivec3 block_pos, bool rec) {
    if (state < BlockArrayInitialized) return 0;
    if (off_bounds(block_pos)) {
        if (rec)
            return chunk_manager->getLightValue({block_pos.x + pos.x * chunk_size.x,
                                                 block_pos.y,
                                                 block_pos.z + pos.y * chunk_size.z});
        return 0b11111111;
    }
    return lightMap[index(block_pos)];
}

void Chunk::render(GLuint program) {
    if (state != Ready) {
        if (state == BlockArrayInitialized)
            generateLightMap();
        if (state == LightMapGenerated)
            build_mesh();
        if (state == MeshBuilt)
            send_mesh_to_gpu();
    }
    if (state != Ready) return;
    setUniform(program, "u_chunkPos", glm::ivec3(pos.x, 0, pos.y));
    chunk_mesh.mesh->render();
}
