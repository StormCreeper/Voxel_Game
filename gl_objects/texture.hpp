/*
    texture.hpp
    author: Telo PHILIPPE

    A Texture class, to load and bind textures, with proper deletion.
*/

#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include "../utils/gl_includes.hpp"

class Texture {
   public:
    Texture() {}
    Texture(const std::string &filename) {
        load(filename);
    }

    void load(const std::string &filename);
    void bind() const;
    void unbind() const;
    inline GLuint getID() const { return m_textureID; }

    ~Texture();

   private:
    GLuint m_textureID = 0;
};

#endif  // TEXTURE_H