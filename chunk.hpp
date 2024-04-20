#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "object3d.hpp"
#include <iostream>

class ChunkManager;

const float tex_num_x = 4;
const float tex_num_y = 1;

enum DIR {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

class Chunk : public Object3D {
   public:
    static const glm::ivec3 chunk_size;
    static std::shared_ptr<Texture> chunk_texture;

    static void init_chunks() {
        chunk_texture = std::make_shared<Texture>("../resources/media/atlas.jpg");
    }

   public:
    Chunk(glm::ivec2 pos, ChunkManager *chunk_manager) {
        this->chunk_manager = chunk_manager;
        // std::cout << "Chunk created at (" << pos.x << ", " << pos.y << ")" << std::endl;
        this->pos = pos;
        this->modelMatrix = glm::translate(modelMatrix, glm::vec3(pos.x * chunk_size.x, 0, pos.y * chunk_size.z));
        this->texture = chunk_texture;

        mesh->genBuffers();
    }

    ~Chunk() {
        free_mem();
    }

    void build_mesh();

    void allocate() {
        voxelMap = (uint8_t *)malloc(chunk_size.x * chunk_size.y * chunk_size.z * sizeof(uint8_t));
        if (!voxelMap) {
            std::cout << "NOOOOOOO no room left :( youre computer is ded :(\n";
            exit(-1);
        }

        std::cout << "Allocated new chunk at (" << pos.x << ", " << pos.y << ")\n";
        allocated = true;
    }

    void free_mem() {
        if (voxelMap)
            free(voxelMap);
        std::cout << "Freed chunk at (" << pos.x << ", " << pos.y << ")\n";
        allocated = false;
    }
    void voxel_map_from_noise();

    uint8_t getBlock(int x, int y, int z, bool rec = true);

   private:
    inline int index(int x, int y, int z) const {
        return x + y * chunk_size.x + z * chunk_size.x * chunk_size.y;
    }

    int push_vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv);

    void push_face(DIR dir, int texIndex);

   public:
    uint8_t *voxelMap{};

   private:
    std::vector<float> vp{};
    std::vector<float> vn{};
    std::vector<float> vuv{};

    glm::vec3 world_offset{};
    glm::vec2 tex_offset{};
    glm::vec2 tex_size{1, 1};

    int vert_index{};
    glm::ivec2 pos{};

    bool allocated = false;
    ChunkManager *chunk_manager;

    int current_block = 0;
};

#endif  // CHUNK_HPP