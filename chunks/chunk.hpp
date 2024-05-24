#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <mutex>
#include <atomic>

#include "../gl_objects/mesh.hpp"
#include <iostream>
#include "../block_palette.hpp"

enum ChunkState {
    EmptyChunk,
    Allocated,
    BlockArrayInitialized,
    LightMapGenerated,
    MeshBuilt,
    Ready
};

class ChunkManager;

const int tex_num_x = 8;
const int tex_num_y = 2;

struct ChunkMesh {
    std::vector<GLuint> vp{};
    std::vector<float> vn{};
    std::vector<float> vuv{};

    std::shared_ptr<Mesh> mesh{};
    glm::mat4 modelMatrix = glm::mat4(1.0f);
};

class Chunk {
   public:
    static inline constexpr glm::ivec3 chunk_size = {16, 128, 16};
    static constexpr inline const int num_blocks = chunk_size.x * chunk_size.y * chunk_size.z;

    static std::shared_ptr<Texture> chunk_texture;

   public:
    uint8_t *voxelMap{};
    bool hasBeenModified = false;
    glm::ivec2 pos{};

    std::atomic<ChunkState> state = EmptyChunk;
    std::atomic<bool> concurrent_use = false;
    std::atomic<bool> out_of_thread = false;
    std::mutex chunk_mutex;

   private:
    ChunkMesh chunk_mesh;
    uint8_t *lightMap{};

    glm::ivec3 world_offset{};
    glm::vec2 tex_offset{};
    glm::vec2 tex_size{1, 1};
    float light_level = 0;

    ChunkManager *chunk_manager;

   public:
    static void init_chunks() {
        BlockPalette::init_block_descs();
    }
    /**
     * @brief Constructor for Chunk class.
     * @param pos Position of the chunk in chunk coordinates.
     * @param chunk_manager Pointer to the parent ChunkManager.
     */
    Chunk(glm::ivec2 pos, ChunkManager *chunk_manager);

    void init(glm::ivec2 pos);

    /// @brief Destructor for Chunk class.
    ~Chunk() {
        free_mem();
    }

    /// @brief Builds (or rebuilds) the chunk mesh based on the voxel grid
    void build_mesh();

    void send_mesh_to_gpu();

    void generateLightMap();
    void floodFill(glm::ivec3 block_pos, uint8_t value, bool sky, bool first = false);

    /// @brief Allocates memory for the chunk's voxel data
    void allocate();

    void free_mem();

    /// @brief generate a voxel map using different noise functions
    void voxel_map_from_noise();

    /**
     * @brief Gets a block ID in the chunk array. If the @param rec flag is set and the block exceed the chunk's bounds, look in neighbouring chunks.
     * @param block_pos position of the block in local space
     * @param rec true to allows looking in neighbouring chunks
     * @return the block's ID
     */
    uint8_t getBlock(glm::ivec3 block_pos, bool rec = true);

    /**
     * @brief Sets a block without rebuilding the mesh
     * @param block_pos
     * @param block
     */
    void setBlock(glm::ivec3 block_pos, uint8_t block);

    uint8_t get_light_value(glm::ivec3 block_pos, bool rec = true);

    inline void set_sky_light(glm::ivec3 block_pos, uint8_t value) {
        if (off_bounds(block_pos)) return;
        lightMap[index(block_pos)] = (value << 4) + (lightMap[index(block_pos)] & 0b00001111);
    }

    /**
     * @brief Renders the chunk
     * @param program the shader program id
     */
    void render(GLuint program);

   private:
    /**
     * @brief Calculates the index in the chunk array for a given local space position.
     * @param pos Position in local space.
     * @return Index in the chunk array.
     */
    inline int index(glm::ivec3 pos) const {
        return pos.x * chunk_size.x * chunk_size.y + pos.y * chunk_size.x + pos.z;
    }

    inline bool off_bounds(glm::ivec3 pos) const {
        return pos.x < 0 || pos.y < 0 || pos.z < 0 ||
               pos.x >= chunk_size.x || pos.y >= chunk_size.y || pos.z >= chunk_size.z;
    }

    /**
     * @brief Pushes a vertex into the mesh arrays.
     * @param pos Vertex position.
     * @param norm Vertex normal.
     * @param uv Vertex UV coordinates.
     */
    void push_vertex(glm::ivec3 pos, glm::vec2 uv);

    /**
     * @brief Pushes a face into the mesh arrays, in the right direction and accounting for the offsets.
     * @param dir Direction of the face.
     * @param texIndex Texture index.
     */
    void push_face(DIR dir, int texIndex);
};

#endif  // CHUNK_HPP