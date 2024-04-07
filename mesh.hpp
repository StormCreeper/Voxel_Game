/*
    mesh.hpp
    author: Telo PHILIPPE

    A Mesh class, to load and render meshes, with proper deletion.
    Also a function to generate a cube sphere mesh.

*/

#ifndef MESH_H
#define MESH_H

#include "gl_includes.hpp"

#include <memory>
#include <vector>

class Mesh {
public:
    void initGPUGeometry(const std::vector<float> &vertexPositions, const std::vector<float> &vertexNormals, const std::vector<float> &vertexUVs, const std::vector<unsigned int> &triangleIndices);
    void setGPUGeometry(GLuint posVbo, GLuint normalVbo, GLuint uvVbo, GLuint ibo, GLuint vao, size_t numIndices);
    void render() const;

    ~Mesh();
    
private:
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_uvVbo = 0;
    GLuint m_ibo = 0;

    size_t m_numIndices = 0;
};


#endif // MESH_H