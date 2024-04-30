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
#include <mutex>

struct cmpChunkPos {
    inline bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        if (a.x == b.x) return a.y < b.y;
        return a.x < b.x;
    }
};

struct cmpChunkPosOrigin {
    static inline glm::vec2 center{};

    inline bool operator()(const std::shared_ptr<Chunk>& a, const std::shared_ptr<Chunk>& b) {
        glm::vec2 diff_a = center - (glm::vec2(a->pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
        glm::vec2 diff_b = center - (glm::vec2(b->pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

        return glm::length(diff_a) < glm::length(diff_b);
    }
};

class ChunkManager {
   public:
    ChunkManager() {}
    ~ChunkManager() {
        saveChunks();
    }

    glm::vec2 chunk_center(glm::ivec2 chunk_pos) {
        return (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
    }

    float chunk_distance(glm::ivec2 chunk_pos, glm::vec3 cam_pos) {
        return glm::length(glm::vec2(cam_pos.x, cam_pos.z) - chunk_center(chunk_pos));
    }

    void updateQueue(glm::vec3 world_pos) {
        glm::ivec2 chunk_pos_center = glm::ivec2((world_pos.x - Chunk::chunk_size.x / 2) / Chunk::chunk_size.x, (world_pos.z - Chunk::chunk_size.z / 2) / Chunk::chunk_size.z);
        for (int i = -load_distance; i <= load_distance; i++) {
            for (int j = -load_distance; j <= load_distance; j++) {
                glm::ivec2 chunk_pos = glm::ivec2(i, j) + chunk_pos_center;
                float dist = chunk_distance(chunk_pos, world_pos);
                if (dist >= load_distance * Chunk::chunk_size.x) continue;

                map_mutex.lock();
                std::shared_ptr<Chunk> chunk{};
                auto it = chunks.find(chunk_pos);
                if (it == chunks.end()) {
                    chunk = std::make_shared<Chunk>(chunk_pos, this);
                    chunks.insert_or_assign(chunk_pos, chunk);
                    taskQueue.push_front(chunk);
                }
                map_mutex.unlock();
            }
        }

        cmpChunkPosOrigin::center = glm::vec2(world_pos.x, world_pos.z);
        std::sort(taskQueue.begin(), taskQueue.end(), cmpChunkPosOrigin());
    }

    void unloadUselessChunks(glm::vec3 cam_pos) {
        map_mutex.lock();
        auto it = chunks.begin();
        while (it != chunks.end()) {
            glm::ivec2 chunk_pos = it->first;
            float dist = chunk_distance(chunk_pos, cam_pos);
            if (dist >= unload_distance * Chunk::chunk_size.x) {
                auto tmp_it = it;
                ++it;
                if (tmp_it->second->hasBeenModified) {
                    serializeChunk(tmp_it->first);
                }
                chunks.erase(tmp_it);
            } else
                ++it;
        }
        map_mutex.unlock();
    }

    void regenerateOneChunkMesh(glm::ivec2 chunk_pos) {
        map_mutex.lock();
        if (auto search = chunks.find(chunk_pos); search != chunks.end()) {
            search->second->meshGenerated = false;
        }
        map_mutex.unlock();
    }

    std::shared_ptr<Chunk> getChunkFromQueue(glm::ivec3 cam_pos) {
        bool found_one = false;
        std::shared_ptr<Chunk> chunk{};

        while (!found_one) {
            if (taskQueue.empty()) return nullptr;

            chunk = taskQueue.front();
            taskQueue.pop_front();

            float dist = chunk_distance(chunk->pos, cam_pos);
            if (dist >= unload_distance * Chunk::chunk_size.x) continue;

            map_mutex.lock();
            if (chunks.find(chunk->pos) != chunks.end()) found_one = true;
            map_mutex.unlock();
        }

        return chunk;
    }

    void generateOrLoadOneChunk(glm::vec3 cam_pos) {
        auto chunk = getChunkFromQueue(cam_pos);
        if (!chunk) return;

        std::cout << "Load or generate one chunk at (" << chunk->pos.x << ", " << chunk->pos.y << ")\n";

        if (!deserializeChunk(chunk)) {
            if (chunk) {
                chunk->voxel_map_from_noise();
            } else {
                std::cout << "Noooooo chunk creation failed :(((((\n";
                exit(-1);
            }
        }
        chunk->meshGenerated = false;
        chunk->set_ready(true);

        regenerateOneChunkMesh(chunk->pos + glm::ivec2(1, 0));
        regenerateOneChunkMesh(chunk->pos + glm::ivec2(-1, 0));
        regenerateOneChunkMesh(chunk->pos + glm::ivec2(0, 1));
        regenerateOneChunkMesh(chunk->pos + glm::ivec2(0, -1));
    }

    void reloadChunks() {
        map_mutex.lock();
        chunks.clear();
        map_mutex.unlock();
        std::cout << "Cleared all chunks\n";
    }

    void saveChunks() {
        map_mutex.lock();
        for (const auto& [pos, chunk] : chunks) {
            if (chunk->is_ready() && chunk->hasBeenModified)
                serializeChunk(pos);
        }
        map_mutex.unlock();
    }

    /**
     * @brief Calculates the angle between two 2D vectors.
     *
     * @param v1 The first vector.
     * @param v2 The second vector.
     * @return The angle between the vectors in radians.
     */
    float calculateAngle(glm::vec2 v1, glm::vec2 v2) {
        return atan2(v1.x * v2.y - v1.y * v2.x, v1.x * v2.x + v1.y * v2.y);
    }

    /// @brief 2D Frustum culling
    bool isInFrustrum(glm::ivec2 chunk_pos, glm::vec3 cam_pos, glm::vec2 cam_dir, float fov) {
        if (chunk_distance(chunk_pos, cam_pos) >= view_distance * Chunk::chunk_size.x)
            return false;

        glm::vec2 chunk_center_front = chunk_center(chunk_pos) + cam_dir * (float)(Chunk::chunk_size.x * sqrt(2));

        glm::vec2 chunk_dir = chunk_center_front - glm::vec2(cam_pos.x, cam_pos.z);

        float angle_front = calculateAngle(cam_dir, chunk_dir);

        return abs(angle_front) < fov / 2.0f;
    }

    /// @todo project cam pos and cam_dir to do 3D frustum culling using 2D
    void renderAll(GLuint program, Camera& camera) {
        glm::vec3 cam_pos = camera.get_position();
        glm::vec3 cam_target = camera.get_target();
        glm::vec2 cam_dir = glm::normalize(glm::vec2(cam_target.x - cam_pos.x, cam_target.z - cam_pos.z));

        map_mutex.lock();
        for (const auto& [pos, chunk] : chunks) {
            if (chunk->is_ready() && isInFrustrum(pos, cam_pos, cam_dir, glm::radians(180.f))) {
                map_mutex.unlock();
                chunk->render(program);
                map_mutex.lock();
            }
        }
        map_mutex.unlock();
    }

    /// @brief Saves a chunk to a save file by just dumping the voxel data in binary mode
    /// @param chunk_pos the pos of the chunk to save
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

    bool deserializeChunk(std::shared_ptr<Chunk> chunk) {
        std::stringstream ss{};
        ss << "../map_data/" << chunk->pos.x << "." << chunk->pos.y << ".txt";

        std::ifstream myfile;
        myfile.open(ss.str(), std::ios::binary);

        if (!myfile.is_open()) {
            return false;
        }

        size_t size = Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char);

        chunk->allocate();
        myfile.read((char*)chunk->voxelMap, size);

        myfile.close();

        return true;
    }

    /// @brief Gets a block in world space -> chooses the right chunk and right offset
    /// @param world_pos the block pos in world space
    /// @return the id of the block if found, 0 in any other case
    uint8_t getBlock(glm::ivec3 world_pos) {
        glm::ivec2 chunk_pos = glm::ivec2(
            floor(world_pos.x / (float)Chunk::chunk_size.x),
            floor(world_pos.z / (float)Chunk::chunk_size.z));

        glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

        map_mutex.lock();
        auto search = chunks.find(chunk_pos);
        auto end = chunks.end();
        map_mutex.unlock();

        if (search != end) {
            return search->second->getBlock({chunk_coords.x, world_pos.y, chunk_coords.y}, false);
        } else
            return 0;
    }

    /// @brief Sets a block in world space -> chooses the right chunk and right offset
    /// @param world_pos the block pos in world space
    /// @param block the id of the block to place
    /// @param rebuild true to rebuild the mesh of the chunk and the surrounding chunks if needed
    void setBlock(glm::ivec3 world_pos, uint8_t block, bool rebuild) {
        glm::ivec2 chunk_pos = glm::ivec2(
            floor(world_pos.x / (float)Chunk::chunk_size.x),
            floor(world_pos.z / (float)Chunk::chunk_size.z));

        glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

        map_mutex.lock();
        auto search = chunks.find(chunk_pos);
        auto end = chunks.end();
        map_mutex.unlock();

        if (search != end) {
            search->second->setBlock({chunk_coords.x, world_pos.y, chunk_coords.y}, block);
            if (rebuild) {
                search->second->build_mesh();
                if (chunk_coords.x == 0) regenerateOneChunkMesh(chunk_pos + glm::ivec2(-1, 0));
                if (chunk_coords.x == Chunk::chunk_size.x - 1) regenerateOneChunkMesh(chunk_pos + glm::ivec2(1, 0));
                if (chunk_coords.y == 0) regenerateOneChunkMesh(chunk_pos + glm::ivec2(0, -1));
                if (chunk_coords.y == Chunk::chunk_size.z - 1) regenerateOneChunkMesh(chunk_pos + glm::ivec2(0, 1));
            }
        }
    }

    bool raycast(glm::vec3 origin, glm::vec3 direction, int nSteps, glm::ivec3& block_pos, glm::ivec3& normal) {
        block_pos.x = int(floor(origin.x));
        block_pos.y = int(floor(origin.y));
        block_pos.z = int(floor(origin.z));

        float sideDistX;
        float sideDistY;
        float sideDistZ;

        float deltaDX = abs(1 / direction.x);
        float deltaDY = abs(1 / direction.y);
        float deltaDZ = abs(1 / direction.z);
        float perpWallDist = -1;

        int stepX;
        int stepY;
        int stepZ;

        int hit = 0;
        int side;

        if (direction.x < 0) {
            stepX = -1;
            sideDistX = (origin.x - block_pos.x) * deltaDX;
        } else {
            stepX = 1;
            sideDistX = (float(block_pos.x) + 1.0 - origin.x) * deltaDX;
        }
        if (direction.y < 0) {
            stepY = -1;
            sideDistY = (origin.y - block_pos.y) * deltaDY;
        } else {
            stepY = 1;
            sideDistY = (float(block_pos.y) + 1.0 - origin.y) * deltaDY;
        }
        if (direction.z < 0) {
            stepZ = -1;
            sideDistZ = (origin.z - block_pos.z) * deltaDZ;
        } else {
            stepZ = 1;
            sideDistZ = (float(block_pos.z) + 1.0 - origin.z) * deltaDZ;
        }

        int step = 1;

        for (int i = 0; i < nSteps; i++) {
            if (sideDistX < sideDistY && sideDistX < sideDistZ) {
                sideDistX += deltaDX;
                block_pos.x += stepX * step;
                side = 0;
            } else if (sideDistY < sideDistX && sideDistY < sideDistZ) {
                sideDistY += deltaDY;
                block_pos.y += stepY * step;
                side = 1;
            } else {
                sideDistZ += deltaDZ;
                block_pos.z += stepZ * step;
                side = 2;
            }

            uint8_t block = getBlock(block_pos);
            // std::cout << "pos: (" << block_pos.x << ", " << block_pos.y << ", " << block_pos.z << "), block: " << (int)block << "\n";
            if (block != 0) {
                if (side == 0) {
                    perpWallDist = (block_pos.x - origin.x + (1 - stepX * step) / 2) / direction.x;
                    normal = glm::ivec3(1, 0, 0) * -stepX;
                } else if (side == 1) {
                    perpWallDist = (block_pos.y - origin.y + (1 - stepY * step) / 2) / direction.y;
                    normal = glm::ivec3(0, 1, 0) * -stepY;
                } else {
                    perpWallDist = (block_pos.z - origin.z + (1 - stepZ * step) / 2) / direction.z;
                    normal = glm::ivec3(0, 0, 1) * -stepZ;
                }

                break;
            }
        }
        return perpWallDist >= 0;
    }

    std::map<glm::ivec2, std::shared_ptr<Chunk>, cmpChunkPos> chunks{};
    std::mutex map_mutex{};

   private:
    std::deque<std::shared_ptr<Chunk>> taskQueue{};
    int view_distance = 5;
    int load_distance = 7;
    int unload_distance = 10;
};

#endif  // CHUNK_MANAGER_HPP