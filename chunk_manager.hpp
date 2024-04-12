#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "chunk.hpp"
#include <memory>
#include <map>
#include <queue>
#include <fstream>
#include <sstream>

struct cmpChunkPos {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        if (a.x == b.x) return a.y < b.y;
        return a.x < b.x;
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

        auto chunk = std::shared_ptr<Chunk>(new Chunk(chunk_pos));
        if (!chunk) {
            std::cout << "Noooooo chunk creation failed :(((((\n";
            exit(-1);
        }
        chunk->init();

        chunks.insert_or_assign(chunk_pos, chunk);
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
        }
    }

    std::shared_ptr<Chunk> deserializeChunk(glm::ivec2 chunk_pos) {
        std::stringstream ss{};
        ss << "../map_data/" << chunk_pos.x << "." << chunk_pos.y << ".txt";

        std::ifstream myfile;
        myfile.open(ss.str(), std::ios::binary);

        if (!myfile.is_open()) {
            std::cerr << "Oh no !! Couldn't read chunk file :(\n";
            return nullptr;
        }

        size_t size = Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char);

        auto chunk = std::make_shared<Chunk>(chunk_pos);
        chunk->voxelMap = (uint8_t*)malloc(Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(uint8_t));
        myfile.read((char*)chunk->voxelMap, size);

        myfile.close();

        chunk->init_no_generate();

        std::cout << "Loaded chunk :D\n";

        return chunk;
    }

    std::map<glm::ivec2, std::shared_ptr<Chunk>, cmpChunkPos> chunks{};

   private:
    std::queue<glm::ivec2> taskQueue{};
};

#endif  // CHUNK_MANAGER_HPP