#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "chunk.hpp"
#include <memory>
#include <map>
#include <deque>
#include <algorithm>
#include <fstream>
#include <sstream>

struct cmpChunkPos {
    inline bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        if (a.x == b.x) return a.y < b.y;
        return a.x < b.x;
    }
};

struct cmpChunkPosOrigin {
    static inline glm::vec2 center{};

    inline bool operator()(const glm::ivec2& a, const glm::ivec2& b) {
        glm::vec2 diff_a = center - (glm::vec2(a) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
        glm::vec2 diff_b = center - (glm::vec2(b) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

        return glm::length(diff_a) < glm::length(diff_b);
    }
};

class ChunkManager {
   public:
    ChunkManager() {}

    void updateQueue(glm::vec3 worldPosition) {
        for (int i = -chunk_view_distance; i <= chunk_view_distance; i++) {
            for (int j = -chunk_view_distance; j <= chunk_view_distance; j++) {
                glm::ivec2 chunk_pos = glm::ivec2(i, j) + glm::ivec2((worldPosition.x - Chunk::chunk_size.x / 2) / Chunk::chunk_size.x, (worldPosition.z - Chunk::chunk_size.z / 2) / Chunk::chunk_size.z);

                if (std::find(taskQueue.begin(), taskQueue.end(), chunk_pos) == taskQueue.end())
                    if (chunks.find(chunk_pos) == chunks.end()) {
                        taskQueue.push_front(chunk_pos);
                    }
            }
        }

        cmpChunkPosOrigin::center = glm::vec2(worldPosition.x, worldPosition.z);
        std::sort(taskQueue.begin(), taskQueue.end(), cmpChunkPosOrigin());
    }

    void regenerateOneChunkMesh(glm::ivec2 chunk_pos) {
        if (auto search = chunks.find(chunk_pos); search != chunks.end()) {
            search->second->build_mesh();
        }
    }

    void generateOrLoadOneChunk() {
        glm::ivec2 chunk_pos{};
        bool found_one = false;

        while (!found_one) {
            if (taskQueue.empty()) return;

            chunk_pos = taskQueue.front();
            taskQueue.pop_front();

            if (auto search = chunks.find(chunk_pos); search == chunks.end()) found_one = true;  // Chunk has already been created for some reason
        }

        auto chunk = deserializeChunk(chunk_pos);

        if (!chunk) {
            chunk = std::shared_ptr<Chunk>(new Chunk(chunk_pos));
            if (chunk) {
                chunk->voxel_map_from_noise();
            } else {
                std::cout << "Noooooo chunk creation failed :(((((\n";
                exit(-1);
            }
        }
        chunk->build_mesh();
        chunks.insert_or_assign(chunk_pos, chunk);

        regenerateOneChunkMesh(chunk_pos + glm::ivec2(1, 0));
        regenerateOneChunkMesh(chunk_pos + glm::ivec2(-1, 0));
        regenerateOneChunkMesh(chunk_pos + glm::ivec2(0, 1));
        regenerateOneChunkMesh(chunk_pos + glm::ivec2(0, -1));
    }

    void reloadChunks() {
        chunks.clear();
        std::cout << "Cleared all chunks\n";
    }

    void saveChunks() {
        for (const auto& [pos, chunk] : chunks) {
            serializeChunk(pos);
        }
    }

    void renderAll(GLuint program) {
        for (const auto& [pos, chunk] : chunks) {
            chunk->render(program);
        }
    }

    void serializeChunk(glm::ivec2 chunk_pos) {
        if (auto search = chunks.find(chunk_pos); search == chunks.end()) {
            std::cout << "Noooooo couldn't write an inexistant chunk to a file\n";
            return;
        }

        std::stringstream ss{};
        ss << "../map_data/" << chunk_pos.x << "." << chunk_pos.y << ".txt";

        std::ofstream myfile;
        myfile.open(ss.str(), std::ios::binary);

        if (!myfile.is_open()) {
            std::cerr << "Error !! Couldn't open file !!\n";
        } else {
            myfile.write((const char*)chunks[chunk_pos]->voxelMap, Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char));
            myfile.close();

            std::cout << "Wrote one chunk at (" << chunk_pos.x << ", " << chunk_pos.y << ")\n";
        }
    }

    std::shared_ptr<Chunk> deserializeChunk(glm::ivec2 chunk_pos) {
        std::stringstream ss{};
        ss << "../map_data/" << chunk_pos.x << "." << chunk_pos.y << ".txt";

        std::ifstream myfile;
        myfile.open(ss.str(), std::ios::binary);

        if (!myfile.is_open()) {
            std::cerr << "Chunk file doesn't exist :(\n";
            return nullptr;
        }

        size_t size = Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char);

        auto chunk = std::make_shared<Chunk>(chunk_pos);
        chunk->allocate();
        myfile.read((char*)chunk->voxelMap, size);

        myfile.close();

        std::cout << "Loaded chunk :D\n";

        return chunk;
    }

    std::map<glm::ivec2, std::shared_ptr<Chunk>, cmpChunkPos> chunks{};

   private:
    std::deque<glm::ivec2> taskQueue{};
    int chunk_view_distance = 5;
};

#endif  // CHUNK_MANAGER_HPP