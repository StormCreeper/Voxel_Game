#ifndef BLOCK_PALETTE_HPP
#define BLOCK_PALETTE_HPP

#include <cstdint>
#include <vector>

enum DIR {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

struct BlockDesc {
    int8_t face_indices[6];
};

class BlockPalette {
   public:
    static inline std::vector<BlockDesc> block_descs{};

    static void init_block_descs() {
        block_descs.push_back({{0, 0, 0, 0, 0, 0}});
        block_descs.push_back({{4, 4, 4, 4, 4, 4}});
        block_descs.push_back({{1, 1, 1, 1, 1, 1}});
        block_descs.push_back({{3, 1, 2, 2, 2, 2}});
    }

    static BlockDesc get_block_desc(uint8_t i) {
        if (i >= block_descs.size()) return block_descs[0];
        return block_descs[i];
    }
};

#endif  // BLOCK_PALETTE_HPP