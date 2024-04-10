/*
    mesh.cpp
    author: Telo PHILIPPE

    Implementation of the Mesh class.
*/

#include "mesh.hpp"

#include <cmath>
#include <iostream>

void Mesh::initGPUGeometry(const std::vector<float> &vertexPositions, const std::vector<float> &vertexNormals, const std::vector<float> &vertexUVs, const std::vector<unsigned int> &triangleIndices) {
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(float) * vertexPositions.size();  // Gather the size of the buffer from the CPU-side vector

    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // Same for the normal vectors
    glGenBuffers(1, &m_normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexNormals.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

    vertexBufferSize = sizeof(float) * vertexUVs.size();

    // Same for the uv coordinates
    glGenBuffers(1, &m_uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexUVs.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(2);

    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the mesh
    size_t indexBufferSize = sizeof(unsigned int) * triangleIndices.size();
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, triangleIndices.data(), GL_DYNAMIC_READ);

    glBindVertexArray(0);  // deactivate the VAO for now, will be activated again when rendering

    m_numIndices = triangleIndices.size();
}

void Mesh::setGPUGeometry(GLuint posVbo, GLuint normalVbo, GLuint uvVbo, GLuint ibo, GLuint vao, size_t numIndices) {
    m_posVbo = posVbo;
    m_normalVbo = normalVbo;
    m_uvVbo = uvVbo;
    m_ibo = ibo;
    m_vao = vao;
    m_numIndices = numIndices;
}

void Mesh::render() const {
    glBindVertexArray(m_vao);                                                    // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);  // Call for rendering: stream the current GPU geometry through the current GPU program
    glBindVertexArray(0);                                                        // deactivate the VAO again 
}

Mesh::~Mesh() {
    if(m_posVbo) glDeleteBuffers(1, &m_posVbo);
    if(m_normalVbo) glDeleteBuffers(1, &m_normalVbo);
    if(m_uvVbo) glDeleteBuffers(1, &m_uvVbo);
    if(m_ibo) glDeleteBuffers(1, &m_ibo);
    
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
}