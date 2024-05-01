#ifndef BLOCK_PALETTE_HPP
#define BLOCK_PALETTE_HPP

#include "texture.hpp"
#include "shader.hpp"

#include <cstdint>
#include <vector>
#include <memory>

#include <fstream>

/// @brief Represents the direction of a face of a cube
enum DIR {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

/// @brief The description of a single block type
struct BlockDesc {
    int8_t face_indices[6];
};

/// @brief A palette of block types, holding an atlas texture and an array of block descriptions
class BlockPalette {
   public:
    static inline std::vector<BlockDesc> block_descs{};
    static inline std::shared_ptr<Texture> texture{};

    static inline glm::ivec3 Normal[] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 0, 0},
        {-1, 0, 0},
        {0, 0, 1},
        {0, 0, -1}};

    static inline float face_light[] = {
        1, 0.5, 0.7, 0.8, 0.9, 0.6};

    static void init_block_descs() {
        texture = std::make_shared<Texture>("../resources/media/atlas.png");

        block_descs.push_back({{0, 0, 0, 0, 0, 0}});

        std::ifstream file;
        file.open("../resources/blocks/block_desc.txt");
        if (!file.is_open()) {
            std::cout << "Couldn't open block description file :(" << std::endl;
            return;
        }

        int faces[6];
        while (file) {
            for (int i = 0; i < 6; i++)
                file >> faces[i];

            block_descs.push_back({});

            std::copy(faces, faces + 6,
                      block_descs[block_descs.size() - 1].face_indices);
        }
    }

    /// @brief Gets a block description in the palette
    /// @param i The block ID
    /// @return the block description
    static inline BlockDesc get_block_desc(uint8_t i) {
        if (i >= block_descs.size()) return block_descs[0];
        return block_descs[i];
    }

    static void inline bind_texture(GLuint program) {
        glActiveTexture(GL_TEXTURE0);
        texture->bind();
        setUniform(program, "u_texture", 0);
    }
};

#endif  // BLOCK_PALETTE_HPP