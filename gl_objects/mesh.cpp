/*
    mesh.cpp
    author: Telo PHILIPPE

    Implementation of the Mesh class.
*/

#include "mesh.hpp"

#include <cmath>
#include <iostream>

void Mesh::genBuffers() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_posVbo);
    glGenBuffers(1, &m_lightingVbo);
    glGenBuffers(1, &m_uvVbo);
}

void Mesh::initGPUGeometry(const std::vector<GLuint> &vertexPositions, const std::vector<float> &vertexLighting, const std::vector<float> &vertexUVs) {
    glBindVertexArray(m_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(GLuint) * vertexPositions.size();  // Gather the size of the buffer from the CPU-side vector

    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexPositions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(GLuint), 0);
    glEnableVertexAttribArray(0);

    vertexBufferSize = sizeof(float) * vertexLighting.size();

    // Same for the lighting values
    glBindBuffer(GL_ARRAY_BUFFER, m_lightingVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexLighting.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

    vertexBufferSize = sizeof(float) * vertexUVs.size();

    // Same for the uv coordinates
    glBindBuffer(GL_ARRAY_BUFFER, m_uvVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexUVs.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    m_numIndices = vertexPositions.size();
}

void Mesh::setGPUGeometry(GLuint posVbo, GLuint normalVbo, GLuint uvVbo, GLuint vao, size_t numIndices) {
    m_posVbo = posVbo;
    m_lightingVbo = normalVbo;
    m_uvVbo = uvVbo;
    m_vao = vao;
    m_numIndices = numIndices;
}

void Mesh::render() const {
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_numIndices);
}

Mesh::~Mesh() {
    if (m_posVbo) glDeleteBuffers(1, &m_posVbo);
    if (m_lightingVbo) glDeleteBuffers(1, &m_lightingVbo);
    if (m_uvVbo) glDeleteBuffers(1, &m_uvVbo);

    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}