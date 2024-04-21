#ifndef CUBE_MAP_HPP
#define CUBE_MAP_HPP

#include "texture.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include <memory>

class CubeMap {
   public:
    CubeMap() {
        // Cube map program
        program = glCreateProgram();
        loadShader(program, GL_VERTEX_SHADER, "../resources/cubeMapVertexShader.glsl");
        loadShader(program, GL_FRAGMENT_SHADER, "../resources/cubeMapFragmentShader.glsl");
        glLinkProgram(program);

        texture = std::make_shared<Texture>("../resources/media/stars.jpg");

        std::vector<float> vertexPositions = {
            // front
            -1.0f, -1.0f, 1.0f,  // 0
            1.0f, 1.0f, 1.0f,    // 2
            1.0f, -1.0f, 1.0f,   // 1
            1.0f, 1.0f, 1.0f,    // 2
            -1.0f, -1.0f, 1.0f,  // 0
            -1.0f, 1.0f, 1.0f,   // 3
            // right
            1.0f, -1.0f, 1.0f,   // 1
            1.0f, 1.0f, -1.0f,   // 6
            1.0f, -1.0f, -1.0f,  // 5
            1.0f, 1.0f, -1.0f,   // 6
            1.0f, -1.0f, 1.0f,   // 1
            1.0f, 1.0f, 1.0f,    // 2
            // back
            -1.0f, 1.0f, -1.0f,   // 7
            1.0f, -1.0f, -1.0f,   // 5
            1.0f, 1.0f, -1.0f,    // 6
            1.0f, -1.0f, -1.0f,   // 5
            -1.0f, 1.0f, -1.0f,   // 7
            -1.0f, -1.0f, -1.0f,  // 4
            // left
            -1.0f, -1.0f, -1.0f,  // 4
            -1.0f, 1.0f, 1.0f,    // 3
            -1.0f, -1.0f, 1.0f,   // 0
            -1.0f, 1.0f, 1.0f,    // 3
            -1.0f, -1.0f, -1.0f,  // 4
            -1.0f, 1.0f, -1.0f,   // 7
            // bottom
            -1.0f, -1.0f, -1.0f,  // 4
            1.0f, -1.0f, 1.0f,    // 1
            1.0f, -1.0f, -1.0f,   // 5
            1.0f, -1.0f, 1.0f,    // 1
            -1.0f, -1.0f, -1.0f,  // 4
            -1.0f, -1.0f, 1.0f,   // 0
            // top
            -1.0f, 1.0f, 1.0f,   // 3
            1.0f, 1.0f, -1.0f,   // 6
            1.0f, 1.0f, 1.0f,    // 2
            1.0f, 1.0f, -1.0f,   // 6
            -1.0f, 1.0f, 1.0f,   // 3
            -1.0f, 1.0f, -1.0f,  // 7

        };

        std::vector<int> triangleIndices = {
            // front
            0, 2, 1,
            2, 0, 3,
            // right
            1, 6, 5,
            6, 1, 2,
            // back
            7, 5, 6,
            5, 7, 4,
            // left
            4, 3, 0,
            3, 4, 7,
            // bottom
            4, 1, 5,
            1, 4, 0,
            // top
            3, 6, 2,
            6, 3, 7};

        GLuint vao;
        GLuint posVbo, ibo;

        // Create a single handle, vertex array object that contains attributes,
        // vertex buffer objects (e.g., vertex's position, normal, and color)
        glGenVertexArrays(1, &vao);  // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.

        glBindVertexArray(vao);

        // Generate a GPU buffer to store the positions of the vertices
        size_t vertexBufferSize = sizeof(float) * vertexPositions.size();  // Gather the size of the buffer from the CPU-side vector

        glGenBuffers(1, &posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, posVbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexPositions.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);

        // Same for an index buffer object that stores the list of indices of the
        // triangles forming the mesh
        size_t indexBufferSize = sizeof(unsigned int) * triangleIndices.size();
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, triangleIndices.data(), GL_DYNAMIC_READ);

        glBindVertexArray(0);  // deactivate the VAO for now, will be activated again when rendering

        size_t numIndices = triangleIndices.size();

        mesh = std::make_shared<Mesh>();
        mesh->setGPUGeometry(posVbo, 0, 0, vao, numIndices);
    }

    ~CubeMap() {
        glDeleteProgram(program);
    }

    void render(glm::mat4 const &projMatrix, glm::mat4 const &viewMatrix) {
        glDepthMask(GL_FALSE);
        glUseProgram(program);

        setUniform(program, "u_viewMat", glm::mat4(glm::mat3(viewMatrix)));
        setUniform(program, "u_projMat", projMatrix);

        setUniform(program, "u_texture", 0);

        glActiveTexture(GL_TEXTURE0);
        texture->bind();

        mesh->render();

        glDepthMask(GL_TRUE);
    }

   private:
    GLuint program{};
    std::shared_ptr<Texture> texture{};
    std::shared_ptr<Mesh> mesh{};
};

#endif  // CUBE_MAP_HPP