/*
    mesh.hpp
    author: Telo PHILIPPE

    A Mesh class, to load and render meshes, with proper deletion.
    Also a function to generate a cube sphere mesh.

*/

#ifndef MESH_H
#define MESH_H

#include "../gl_includes.hpp"

#include <memory>
#include <vector>

class Mesh {
   public:
    void genBuffers();
    void initGPUGeometry(const std::vector<GLuint> &vertexPositions, const std::vector<float> &vertexLighting, const std::vector<float> &vertexUVs);
    void setGPUGeometry(GLuint posVbo, GLuint lightingVbo, GLuint uvVbo, GLuint vao, size_t numIndices);
    void render() const;

    ~Mesh();

   private:
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_lightingVbo = 0;
    GLuint m_uvVbo = 0;

    size_t m_numIndices = 0;
};

#endif  // MESH_H