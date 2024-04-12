#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "chunk.hpp"
#include <memory>
#include <map>
#include <queue>

class ChunkManager {
   public:
    ChunkManager() {}
    void updateQueue(glm::vec3 worldPosition) {
        glm::ivec2 chunk_pos = glm::ivec2(worldPosition.x / Chunk::chunk_size.x, worldPosition.z / Chunk::chunk_size.z);
    }

    void generateOneChunk() {
    }

   private:
    std::map<glm::ivec2, std::shared_ptr<Chunk>> chunks{};
    std::queue<glm::ivec2> taskQueue{};
};

#endif  // CHUNK_MANAGER_HPP