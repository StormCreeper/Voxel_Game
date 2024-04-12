#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "object3d.hpp"
#include <iostream>

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
    Chunk(glm::ivec2 pos) {
        std::cout << "Chunk created at (" << pos.x << ", " << pos.y << ")" << std::endl;
        this->pos = pos;
        this->modelMatrix = glm::translate(modelMatrix, glm::vec3(pos.x * chunk_size.x, 0, pos.y * chunk_size.z));
    }

    ~Chunk() {
        if (voxelMap)
            free(voxelMap);
    }

    void init() {
        generateVoxelMap();
        buildMesh();
        this->texture = chunk_texture;
    }

   private:
    inline int index(int x, int y, int z) const {
        return x + y * chunk_size.x + z * chunk_size.x * chunk_size.y;
    }

    void generateVoxelMap();
    int push_vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv);

    void push_triangle(glm::ivec3 tri);

    void push_face(DIR dir, int texIndex);
    void buildMesh();

    int getBlock(int x, int y, int z);

   private:
    int *voxelMap{};

    std::vector<float> vp{};
    std::vector<float> vn{};
    std::vector<float> vuv{};
    std::vector<unsigned int> ti{};

    glm::vec3 world_offset{};
    glm::vec2 tex_offset{};
    glm::vec2 tex_size{1, 1};

    int vert_index{};
    glm::ivec2 pos{};
};

#endif  // CHUNK_HPP