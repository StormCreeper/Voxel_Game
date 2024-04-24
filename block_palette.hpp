#ifndef BLOCK_PALETTE_HPP
#define BLOCK_PALETTE_HPP

#include "texture.hpp"
#include "shader.hpp"

#include <cstdint>
#include <vector>
#include <memory>

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

    static void init_block_descs() {
        block_descs.push_back({{0, 0, 0, 0, 0, 0}});
        block_descs.push_back({{4, 4, 4, 4, 4, 4}});
        block_descs.push_back({{1, 1, 1, 1, 1, 1}});
        block_descs.push_back({{3, 1, 2, 2, 2, 2}});
        block_descs.push_back({{7, 7, 7, 7, 7, 7}});
        block_descs.push_back({{6, 6, 5, 5, 5, 5}});
        block_descs.push_back({{8, 8, 8, 8, 8, 8}});
        block_descs.push_back({{11, 11, 10, 10, 9, 10}});

        texture = std::make_shared<Texture>("../resources/media/atlas.png");
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