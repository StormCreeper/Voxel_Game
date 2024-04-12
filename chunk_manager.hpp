#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "chunk.hpp"
#include <memory>
#include <map>
#include <queue>

struct cmpChunkPos {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        return (a.x == b.x && a.y < b.y) || a.x < a.y;
    }
};

class ChunkManager {
   public:
    ChunkManager() {}
    void updateQueue(glm::vec3 worldPosition) {
        glm::ivec2 chunk_pos = glm::ivec2(worldPosition.x / Chunk::chunk_size.x, worldPosition.z / Chunk::chunk_size.z);
        if (auto search = chunks.find(chunk_pos); search != chunks.end()) {
        } else {
            taskQueue.push(chunk_pos);
        }
    }

    void generateOneChunk() {
        if (taskQueue.empty()) return;
        glm::ivec2 chunk_pos = taskQueue.front();
        taskQueue.pop();

        if (auto search = chunks.find(chunk_pos); search != chunks.end()) return;  // Chunk has already been created for some reason

        chunks[chunk_pos] = std::make_shared<Chunk>(chunk_pos);
        chunks[chunk_pos]->init();
    }

    void renderAll(GLuint program) {
        for (const auto& [pos, chunk] : chunks) {
            chunk->render(program);
        }
    }

   private:
    std::map<glm::ivec2, std::shared_ptr<Chunk>, cmpChunkPos> chunks{};
    std::queue<glm::ivec2> taskQueue{};
};

#endif  // CHUNK_MANAGER_HPP