#ifndef CUBE_MAP_HPP
#define CUBE_MAP_HPP

#include "gl_objects/texture.hpp"
#include "gl_objects/shader.hpp"

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

        glGenVertexArrays(1, &vao);

        glBindVertexArray(vao);

        size_t vertexBufferSize = sizeof(float) * vertexPositions.size();

        glGenBuffers(1, &pos_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexPositions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        numIndices = vertexPositions.size() / 3;

        glActiveTexture(GL_TEXTURE1);
        texture->bind();
        glActiveTexture(GL_TEXTURE0);

        glUseProgram(program);
        setUniform(program, "u_texture", 1);
    }

    ~CubeMap() {
        glDeleteProgram(program);
    }

    void render(glm::mat4 const &projMatrix, glm::mat4 const &viewMatrix) {
        glDepthMask(GL_FALSE);
        glUseProgram(program);

        setUniform(program, "u_viewMat", glm::mat4(glm::mat3(viewMatrix)));
        setUniform(program, "u_projMat", projMatrix);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, numIndices);

        glDepthMask(GL_TRUE);
    }

   private:
    GLuint program{};
    std::shared_ptr<Texture> texture{};
    GLuint vao, pos_vbo;
    size_t numIndices;
};

#endif  // CUBE_MAP_HPP