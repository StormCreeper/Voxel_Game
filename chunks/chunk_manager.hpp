#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "chunk.hpp"
#include "../camera.hpp"

#include <map>
#include <deque>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <thread>

class ChunkDealer;

/// @brief A struct to compare the position of two Chunks, used for storing them in a map
/// @relates ChunkManager
struct cmpChunkPos {
    inline bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        if (a.x == b.x) return a.y < b.y;
        return a.x < b.x;
    }
};
/// @brief A struct to sort chunk by their distance to the center (the player position). Used to sort the task queue and generating first the chunks that are close.
struct cmpChunkPosOrigin {
    static inline glm::vec2 center{};

    inline bool operator()(const Chunk* a, const Chunk* b) {
        glm::vec2 diff_a = center - (glm::vec2(a->pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
        glm::vec2 diff_b = center - (glm::vec2(b->pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

        return glm::length(diff_a) < glm::length(diff_b);
    }
};

class ChunkManager {
   public:
    ChunkDealer* chunk_dealer;

    std::map<glm::ivec2, Chunk*, cmpChunkPos> chunks{};

    std::mutex queue_mutex{};
    std::condition_variable mutex_condition{};
    std::vector<std::thread> threads;
    bool should_terminate = false;

    glm::vec3 cam_pos;

   private:
    std::deque<Chunk*>
        taskQueue{};

    std::mutex map_mutex{};
    // TODO : change to glm::ivec2
    std::queue<Chunk*> toDelete{};

    bool thread_pool_paused = false;
    int view_distance = 18;
    int load_distance = 20;
    int unload_distance = 23;

   public:  // utility functions
    inline glm::vec2 chunk_center(glm::ivec2 chunk_pos) {
        return (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
    }

    inline float chunk_distance(glm::ivec2 chunk_pos) {
        return glm::length(glm::vec2(cam_pos.x, cam_pos.z) - chunk_center(chunk_pos));
    }

    /**
     * @brief Calculates the angle between two 2D vectors.
     *
     * @param v1 The first vector.
     * @param v2 The second vector.
     * @return The angle between the vectors in radians.
     */
    inline float calculateAngle(glm::vec2 v1, glm::vec2 v2) {
        return atan2(v1.x * v2.y - v1.y * v2.x, v1.x * v2.x + v1.y * v2.y);
    }

    /// @brief 2D Frustum culling
    bool isInFrustrum(glm::ivec2 chunk_pos, glm::vec2 cam_dir, float fov);

   public:
    ChunkManager();

    void destroy();

    void ThreadLoop();

    void updateQueue(glm::vec3 world_pos);

    void unloadUselessChunks();

    void regenerateOneChunkMesh(glm::ivec2 chunk_pos);

    Chunk* getChunkFromQueue();

    void reloadChunks();

    void saveChunks();

    /// @todo project cam pos and cam_dir to do 3D frustum culling using 2D
    void renderAll(GLuint program, Camera& camera);

    /// @brief Saves a chunk to a save file by just dumping the voxel data in binary mode
    /// @param chunk_pos the pos of the chunk to save
    void serializeChunk(glm::ivec2 chunk_pos);

    bool deserializeChunk(Chunk* chunk);

    /// @brief Gets a block in world space -> chooses the right chunk and right offset
    /// @param world_pos the block pos in world space
    /// @return the id of the block if found, 0 in any other case
    uint8_t getBlock(glm::ivec3 world_pos);

    uint8_t getLightValue(glm::ivec3 world_pos);

    /// @brief Sets a block in world space -> chooses the right chunk and right offset
    /// @param world_pos the block pos in world space
    /// @param block the id of the block to place
    /// @param rebuild true to rebuild the mesh of the chunk and the surrounding chunks if needed
    void setBlock(glm::ivec3 world_pos, uint8_t block, bool rebuild);

    bool raycast(glm::vec3 origin, glm::vec3 direction, int nSteps, glm::ivec3& block_pos, glm::ivec3& normal);
};

#endif  // CHUNK_MANAGER_HPP