#include "chunk.hpp"

const glm::ivec3 Chunk::chunk_size = {10, 64, 10};
std::shared_ptr<Texture> Chunk::chunk_texture{};

void Chunk::generateVoxelMap() {
    voxelMap = (uint8_t *)malloc(chunk_size.x * chunk_size.y * chunk_size.z * sizeof(uint8_t));
    if (!voxelMap) {
        std::cout << "NOOOOOOO no room left :( youre computer is ded :(\n";
        exit(-1);
    }

    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                int world_x = x + chunk_size.x * pos.x;
                int world_y = y;
                int world_z = z + chunk_size.z * pos.y;
                voxelMap[index(x, y, z)] = 30 + 9 * (sin((world_x + world_z) * 0.2) * 0.5 + 0.5) > world_y;
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

void Chunk::push_triangle(glm::ivec3 tri) {
    ti.push_back(tri.x);
    ti.push_back(tri.y);
    ti.push_back(tri.z);
}

void Chunk::push_face(DIR dir, int texIndex) {
    tex_offset.x = (float)(texIndex) / tex_num_x;
    tex_offset.y = 0;
    tex_size.x = 1.0 / tex_num_x;
    tex_size.y = 1.0;

    switch (dir) {
        case DIR::UP:
            push_triangle({push_vertex({1, 1, 0}, {0, 1, 0}, {1, 0}),
                           push_vertex({0, 1, 0}, {0, 1, 0}, {0, 0}),
                           push_vertex({1, 1, 1}, {0, 1, 0}, {1, 1})});
            push_triangle({
                push_vertex({1, 1, 1}, {0, 1, 0}, {1, 1}),
                push_vertex({0, 1, 0}, {0, 1, 0}, {0, 0}),
                push_vertex({0, 1, 1}, {0, 1, 0}, {0, 1}),
            });
            break;
        case DIR::DOWN:
            push_triangle({push_vertex({0, 0, 0}, {0, -1, 0}, {0, 0}),
                           push_vertex({1, 0, 0}, {0, -1, 0}, {1, 0}),
                           push_vertex({1, 0, 1}, {0, -1, 0}, {1, 1})});
            push_triangle({push_vertex({0, 0, 0}, {0, -1, 0}, {0, 0}),
                           push_vertex({1, 0, 1}, {0, -1, 0}, {1, 1}),
                           push_vertex({0, 0, 1}, {0, -1, 0}, {0, 1})});
            break;
        case DIR::FRONT:
            push_triangle({push_vertex({0, 0, 1}, {0, 0, 1}, {0, 1}),
                           push_vertex({1, 0, 1}, {0, 0, 1}, {1, 1}),
                           push_vertex({1, 1, 1}, {0, 0, 1}, {1, 0})});
            push_triangle({push_vertex({0, 0, 1}, {0, 0, 1}, {0, 1}),
                           push_vertex({1, 1, 1}, {0, 0, 1}, {1, 0}),
                           push_vertex({0, 1, 1}, {0, 0, 1}, {0, 0})});
            break;
        case DIR::BACK:
            push_triangle({push_vertex({1, 0, 0}, {0, 0, -1}, {1, 1}),
                           push_vertex({0, 0, 0}, {0, 0, -1}, {0, 1}),
                           push_vertex({1, 1, 0}, {0, 0, -1}, {1, 0})});
            push_triangle({push_vertex({1, 1, 0}, {0, 0, -1}, {1, 0}),
                           push_vertex({0, 0, 0}, {0, 0, -1}, {0, 1}),
                           push_vertex({0, 1, 0}, {0, 0, -1}, {0, 0})});
            break;
        case DIR::RIGHT:
            push_triangle({push_vertex({0, 1, 0}, {-1, 0, 0}, {0, 0}),
                           push_vertex({0, 0, 0}, {-1, 0, 0}, {0, 1}),
                           push_vertex({0, 1, 1}, {-1, 0, 0}, {1, 0})});
            push_triangle({push_vertex({0, 1, 1}, {-1, 0, 0}, {1, 0}),
                           push_vertex({0, 0, 0}, {-1, 0, 0}, {0, 1}),
                           push_vertex({0, 0, 1}, {-1, 0, 0}, {1, 1})});
            break;
        case DIR::LEFT:
            push_triangle({push_vertex({1, 0, 0}, {1, 0, 0}, {0, 1}),
                           push_vertex({1, 1, 0}, {1, 0, 0}, {0, 0}),
                           push_vertex({1, 1, 1}, {1, 0, 0}, {1, 0})});
            push_triangle({push_vertex({1, 0, 0}, {1, 0, 0}, {0, 1}),
                           push_vertex({1, 1, 1}, {1, 0, 0}, {1, 0}),
                           push_vertex({1, 0, 1}, {1, 0, 0}, {1, 1})});
            break;
    }
}

void Chunk::buildMesh() {
    vp.clear();
    vn.clear();
    vuv.clear();
    ti.clear();

    vert_index = 0;

    for (int x = 0; x < chunk_size.x; x++) {
        for (int y = 0; y < chunk_size.y; y++) {
            for (int z = 0; z < chunk_size.z; z++) {
                world_offset = {x, y, z};
                if (getBlock(x, y, z)) {
                    if (!getBlock(x, y, z + 1)) push_face(DIR::FRONT, 1);
                    if (!getBlock(x, y, z - 1)) push_face(DIR::BACK, 1);
                    if (!getBlock(x, y - 1, z)) push_face(DIR::DOWN, 0);
                    if (!getBlock(x, y + 1, z)) push_face(DIR::UP, 2);
                    if (!getBlock(x + 1, y, z)) push_face(DIR::LEFT, 1);
                    if (!getBlock(x - 1, y, z)) push_face(DIR::RIGHT, 1);
                }
            }
        }
    }

    std::cout << vp.size() << ", " << vn.size() << ", " << vuv.size() << ", " << ti.size() << "\n";

    mesh->initGPUGeometry(vp, vn, vuv, ti);
}

uint8_t Chunk::getBlock(int x, int y, int z) {
    if (x < 0 || y < 0 || z < 0) return 0;
    if (x >= chunk_size.x || y >= chunk_size.y || z >= chunk_size.z) return 0;

    return voxelMap[index(x, y, z)];
}