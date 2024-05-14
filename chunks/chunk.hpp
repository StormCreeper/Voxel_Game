#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <mutex>
#include <atomic>

#include "../gl_objects/mesh.hpp"
#include <iostream>
#include "../block_palette.hpp"

class ChunkManager;

const int tex_num_x = 8;
const int tex_num_y = 2;

class Chunk {
   public:
    static inline constexpr glm::ivec3 chunk_size = {16, 64, 16};
    static constexpr inline const int num_blocks = chunk_size.x * chunk_size.y * chunk_size.z;

    static std::shared_ptr<Texture> chunk_texture;
    bool meshGenerated = false;
    bool lightMapGenerated = false;

    static void init_chunks() {
        BlockPalette::init_block_descs();
    }

   public:
    /**
     * @brief Constructor for Chunk class.
     * @param pos Position of the chunk in chunk coordinates.
     * @param chunk_manager Pointer to the parent ChunkManager.
     */
    Chunk(glm::ivec2 pos, ChunkManager *chunk_manager) {
        mesh = std::make_shared<Mesh>();
        this->chunk_manager = chunk_manager;
        this->pos = pos;
        this->modelMatrix = glm::translate(modelMatrix, glm::vec3(pos.x * chunk_size.x, 0, pos.y * chunk_size.z));

        mesh->genBuffers();

        // std::cout << "Init chunk at (" << pos.x << ", " << pos.y << ")\n";
    }

    /// @brief Destructor for Chunk class.
    ~Chunk() {
        free_mem();
    }

    /// @brief Builds (or rebuilds) the chunk mesh based on the voxel grid
    void build_mesh();

    void generateLightMap();
    void floodFill(glm::ivec3 block_pos, uint8_t value, bool sky, bool first = false);

    /// @brief Allocates memory for the chunk's voxel data
    void allocate() {
        voxelMap = (uint8_t *)realloc(voxelMap, num_blocks * sizeof(uint8_t));
        lightMap = (uint8_t *)realloc(lightMap, num_blocks * sizeof(uint8_t));
        if (!voxelMap || !lightMap) {
            std::cout << "NOOOOOOO no room left :( youre computer is ded :(\n";
            exit(-1);
        }

        // std::cout << "Allocated new chunk at (" << pos.x << ", " << pos.y << ")\n";
        allocated = true;
    }

    void free_mem() {
        if (voxelMap)
            free(voxelMap);
        if (lightMap)
            free(lightMap);
        // std::cout << "Freed chunk at (" << pos.x << ", " << pos.y << ")\n";
        allocated = false;
    }

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
    void render(GLuint program) {
        if (!lightMapGenerated) {
            generateLightMap();
            meshGenerated = false;
        }
        if (!meshGenerated) {
            build_mesh();
        }
        setUniform(program, "u_chunkPos", glm::ivec3(pos.x, 0, pos.y));
        mesh->render();
    }

    bool is_ready() {
        return ready;
    }

    void set_ready(bool r) {
        ready = r;
    }

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

   public:
    uint8_t *voxelMap{};
    bool hasBeenModified = false;
    glm::ivec2 pos{};

   private:
    std::vector<GLuint> vp{};
    std::vector<float> vn{};
    std::vector<float> vuv{};

    uint8_t *lightMap{};

    glm::ivec3 world_offset{};
    glm::vec2 tex_offset{};
    glm::vec2 tex_size{1, 1};
    float light_level = 0;

    bool allocated = false;
    ChunkManager *chunk_manager;

    std::shared_ptr<Mesh> mesh{};
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    std::atomic_bool ready = false;
};

#endif  // CHUNK_HPP