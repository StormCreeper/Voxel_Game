#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "chunk.hpp"
#include "camera.hpp"

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
        for (int i = -chunk_load_distance; i <= chunk_load_distance; i++) {
            for (int j = -chunk_load_distance; j <= chunk_load_distance; j++) {
                glm::ivec2 chunk_pos = glm::ivec2(i, j) + glm::ivec2((worldPosition.x - Chunk::chunk_size.x / 2) / Chunk::chunk_size.x, (worldPosition.z - Chunk::chunk_size.z / 2) / Chunk::chunk_size.z);
                float dist = glm::length(glm::vec2(worldPosition.x, worldPosition.z) - (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z));
                if (dist >= chunk_load_distance * Chunk::chunk_size.x) continue;

                if (std::find(taskQueue.begin(), taskQueue.end(), chunk_pos) == taskQueue.end())
                    if (chunks.find(chunk_pos) == chunks.end()) {
                        taskQueue.push_front(chunk_pos);
                    }
            }
        }

        cmpChunkPosOrigin::center = glm::vec2(worldPosition.x, worldPosition.z);
        std::sort(taskQueue.begin(), taskQueue.end(), cmpChunkPosOrigin());
    }

    void unloadUselessChunks(glm::vec3 cam_pos) {
        auto it = chunks.begin();
        while (it != chunks.end()) {
            glm::ivec2 chunk_pos = it->first;
            float dist = glm::length(glm::vec2(cam_pos.x, cam_pos.z) - (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z));
            if (dist >= chunk_unload_distance * Chunk::chunk_size.x) {
                auto tmp_it = it;
                ++it;
                chunks.erase(tmp_it);
            } else
                ++it;
        }
    }

    void regenerateOneChunkMesh(glm::ivec2 chunk_pos) {
        if (auto search = chunks.find(chunk_pos); search != chunks.end()) {
            search->second->build_mesh();
        }
    }

    void generateOrLoadOneChunk(glm::vec3 cam_pos) {
        glm::ivec2 chunk_pos{};
        bool found_one = false;

        while (!found_one) {
            if (taskQueue.empty()) return;

            chunk_pos = taskQueue.front();
            taskQueue.pop_front();

            float dist = glm::length(glm::vec2(cam_pos.x, cam_pos.z) - (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z));
            if (dist >= chunk_unload_distance * Chunk::chunk_size.x) continue;

            if (chunks.find(chunk_pos) == chunks.end()) found_one = true;
        }

        auto chunk = deserializeChunk(chunk_pos);

        if (!chunk) {
            chunk = std::shared_ptr<Chunk>(new Chunk(chunk_pos, this));
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
        // TODO
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

    /// @brief 2D Frustum culling
    bool isInFrustrum(glm::ivec2 chunk_pos, glm::vec3 cam_pos, glm::vec2 dir, float fov) {
        float dist = glm::length(glm::vec2(cam_pos.x, cam_pos.z) - (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z));
        if (dist >= chunk_view_distance * Chunk::chunk_size.x) return false;
        glm::vec2 cam_right = glm::normalize(glm::vec2(dir.y, -dir.x));

        glm::vec2 chunk_center = (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
        glm::vec2 chunk_center_front = chunk_center + dir * (float)(Chunk::chunk_size.x * sqrt(2));

        glm::vec2 cam_center = glm::vec2(cam_pos.x, cam_pos.z);

        glm::vec2 chunk_dir_front = chunk_center_front - cam_center;

        float angle_front = atan2(dir.x * chunk_dir_front.y - dir.y * chunk_dir_front.x, dir.x * chunk_dir_front.x + dir.y * chunk_dir_front.y);

        return abs(angle_front) < fov / 2.0f;  // || abs(angle_max) < fov / 2.0f || abs(angle_min + angle_max) / 2.0f < fov / 2.0f;
    }

    /// @todo project cam pos and dir to do 3D frustum culling using 2D
    void renderAll(GLuint program, Camera& camera) {
        glm::vec3 cam_pos = camera.get_position();
        glm::vec3 cam_target = camera.get_target();
        glm::vec2 cam_dir = glm::normalize(glm::vec2(cam_target.x - cam_pos.x, cam_target.z - cam_pos.z));

        BlockPalette::bind_texture(program);

        for (const auto& [pos, chunk] : chunks) {
            if (isInFrustrum(pos, cam_pos, cam_dir, glm::radians(180.f)))
                chunk->render(program);
        }
    }

    void serializeChunk(glm::ivec2 chunk_pos) {
        if (chunks.find(chunk_pos) == chunks.end()) {
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
            // std::cerr << "Chunk file doesn't exist :(\n";
            return nullptr;
        }

        size_t size = Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char);

        auto chunk = std::make_shared<Chunk>(chunk_pos, this);
        chunk->allocate();
        myfile.read((char*)chunk->voxelMap, size);

        myfile.close();

        // std::cout << "Loaded chunk :D\n";

        return chunk;
    }

    uint8_t getBlock(glm::ivec3 world_pos) {
        glm::ivec2 chunk_pos = glm::ivec2(
            floor(world_pos.x / (float)Chunk::chunk_size.x),
            floor(world_pos.z / (float)Chunk::chunk_size.z));

        glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

        if (chunk_coords.x < 0 || chunk_coords.x >= Chunk::chunk_size.x || chunk_coords.y < 0 || chunk_coords.y >= Chunk::chunk_size.z)
            std::cout << "(" << chunk_pos.x << ", " << chunk_pos.y << ", " << chunk_coords.x << ", " << chunk_coords.y << ")\n";

        if (auto search = chunks.find(chunk_pos); search != chunks.end()) {
            return search->second->getBlock(chunk_coords.x, world_pos.y, chunk_coords.y, false);
        } else
            return 0;
    }

    std::map<glm::ivec2, std::shared_ptr<Chunk>, cmpChunkPos> chunks{};

   private:
    std::deque<glm::ivec2> taskQueue{};
    int chunk_view_distance = 5;
    int chunk_load_distance = 7;
    int chunk_unload_distance = 10;
};

#endif  // CHUNK_MANAGER_HPP