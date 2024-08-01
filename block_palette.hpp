#ifndef BLOCK_PALETTE_HPP
#define BLOCK_PALETTE_HPP

#include "gl_objects/texture.hpp"
#include "gl_objects/shader.hpp"

#include <cstdint>
#include <vector>
#include <memory>
#include <map>

#include <filesystem>

#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/// @brief Represents the direction of a face of a cube
enum DIR {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

/// @brief The description of a single block type
struct BlockDesc {
    int8_t face_indices[6];
};

struct NewBlockDesc {
    std::string textures[6];
};

/// @brief A palette of block types, holding an atlas texture and an array of block descriptions
class BlockPalette {
   public:
    static inline std::vector<BlockDesc> block_descs{};
    static inline std::shared_ptr<Texture> texture{};

    static inline std::map<std::string, NewBlockDesc> new_block_descs{};

    static inline std::map<std::string, std::pair<std::shared_ptr<Texture>, GLuint64>> textures;

    static inline glm::ivec3 Normal[] = {
        {0, 1, 0},
        {0, -1, 0},
        {1, 0, 0},
        {-1, 0, 0},
        {0, 0, 1},
        {0, 0, -1}};

    static inline float face_light[] = {
        1, 0.5, 0.7, 0.8, 0.9, 0.6};

    static void init_block_descs() {
        texture = std::make_shared<Texture>("../resources/media/atlas.png");

        block_descs.push_back({{0, 0, 0, 0, 0, 0}});

        std::ifstream file;
        file.open("../resources/blocks/block_desc.txt");
        if (!file.is_open()) {
            std::cout << "Couldn't open block description file :(" << std::endl;
            return;
        }

        int faces[6];
        while (file) {
            for (int i = 0; i < 6; i++)
                file >> faces[i];

            block_descs.push_back({});

            std::copy(faces, faces + 6,
                      block_descs[block_descs.size() - 1].face_indices);
        }

        file.close();

        std::cout << "Block descriptions loaded" << std::endl;

        // Load textures
        load_textures();

        // Load block descriptions
        load_block("grass");
    }

    static void load_textures() {
        std::string folder = "../resources/blocks/textures/";
        std::string extension = ".png";

        std::vector<GLuint64> handles;

        for (const auto &entry : std::filesystem::directory_iterator(folder)) {
            std::string path = entry.path().string();
            std::string name = entry.path().filename().string();

            if (name.find(extension) != std::string::npos) {
                std::shared_ptr<Texture> texture = std::make_shared<Texture>(path);
                // Get texture handle
                const GLuint64 handle = glGetTextureHandleARB(texture->getID());
                if (handle == 0) {
                    std::cout << "Error: texture handle is 0" << std::endl;
                    return;
                }

                std::cout << "Loaded texture: " << name << std::endl;

                textures[name] = {texture, handle};
                handles.push_back(handle);
            }
        }

        GLuint textureBuffer;
        glCreateBuffers(1, &textureBuffer);
        glNamedBufferStorage(
            textureBuffer,
            sizeof(GLuint64) * handles.size(),
            (const void *)handles.data(),
            GL_DYNAMIC_STORAGE_BIT);

        for (const auto &[name, tex] : textures) {
            glMakeTextureHandleResidentARB(tex.second);
        }
    }

    static void load_block(std::string name) {
        std::string path = "../resources/blocks/" + name + ".json";
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "Couldn't open block file :(" << std::endl;
            return;
        }
        json data = json::parse(file);

        NewBlockDesc desc;
        std::string faces[6] = {"up", "down", "left", "right", "front", "back"};
        for (int i = 0; i < 6; i++) {
            desc.textures[i] = data["faces"][faces[i]]["texture"];
        }

        new_block_descs[name] = desc;
    }

    static void destroy_textures() {
        // Textures are automatically destroyed
    }

    /// @brief Gets a block description in the palette
    /// @param i The block ID
    /// @return the block description
    static inline BlockDesc get_block_desc(uint8_t i) {
        if (i >= block_descs.size()) return block_descs[0];
        return block_descs[i];
    }

    static void inline bind_texture(GLuint program) {
        glActiveTexture(GL_TEXTURE0);
        texture->bind();
        setUniform(program, "u_texture", 0);
    }
};

#endif  // BLOCK_PALETTE_HPP