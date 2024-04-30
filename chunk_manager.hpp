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

    inline glm::vec2 chunk_center(glm::ivec2 chunk_pos) {
        return (glm::vec2(chunk_pos) + glm::vec2(0.5, 0.5)) * glm::vec2(Chunk::chunk_size.x, Chunk::chunk_size.z);
    }

    inline float chunk_distance(glm::ivec2 chunk_pos, glm::vec3 cam_pos) {
        return glm::length(glm::vec2(cam_pos.x, cam_pos.z) - chunk_center(chunk_pos));
    }

    void updateQueue(glm::vec3 world_pos);

    void unloadUselessChunks(glm::vec3 cam_pos);

    void regenerateOneChunkMesh(glm::ivec2 chunk_pos);

    std::shared_ptr<Chunk> getChunkFromQueue(glm::ivec3 cam_pos);

    void generateOrLoadOneChunk(glm::vec3 cam_pos);

    void reloadChunks();

    void saveChunks();

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
    bool isInFrustrum(glm::ivec2 chunk_pos, glm::vec3 cam_pos, glm::vec2 cam_dir, float fov);

    /// @todo project cam pos and cam_dir to do 3D frustum culling using 2D
    void renderAll(GLuint program, Camera& camera);

    /// @brief Saves a chunk to a save file by just dumping the voxel data in binary mode
    /// @param chunk_pos the pos of the chunk to save
    void serializeChunk(glm::ivec2 chunk_pos);

    bool deserializeChunk(std::shared_ptr<Chunk> chunk);

    /// @brief Gets a block in world space -> chooses the right chunk and right offset
    /// @param world_pos the block pos in world space
    /// @return the id of the block if found, 0 in any other case
    uint8_t getBlock(glm::ivec3 world_pos);

    /// @brief Sets a block in world space -> chooses the right chunk and right offset
    /// @param world_pos the block pos in world space
    /// @param block the id of the block to place
    /// @param rebuild true to rebuild the mesh of the chunk and the surrounding chunks if needed
    void setBlock(glm::ivec3 world_pos, uint8_t block, bool rebuild);

    bool raycast(glm::vec3 origin, glm::vec3 direction, int nSteps, glm::ivec3& block_pos, glm::ivec3& normal);

    std::map<glm::ivec2, std::shared_ptr<Chunk>, cmpChunkPos> chunks{};
    std::mutex map_mutex{};
    std::mutex queue_mutex{};

   private:
    std::deque<std::shared_ptr<Chunk>> taskQueue{};
    int view_distance = 5;
    int load_distance = 7;
    int unload_distance = 10;
};

#endif  // CHUNK_MANAGER_HPP