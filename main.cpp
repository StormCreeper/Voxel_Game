#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "object3d.hpp"
#include "texture.hpp"
#include "chunk_manager.hpp"

#include "gl_includes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam);

std::shared_ptr<Mesh> g_starsCube{};
std::shared_ptr<Texture> g_starsTexture{};

// Window parameters
GLFWwindow *g_window{};

// GPU objects
GLuint g_program{};  // A GPU program contains at least a vertex shader and a fragment shader

GLuint g_cubeMapProgram{};  // A GPU program for the cube map

Camera g_camera{};

ChunkManager g_chunkManager{};

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height);  // Dimension of the rendering region in the window
}

bool shiftPressed = false;

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    static bool p_unpressed = false;
    static bool o_unpressed = false;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (key == GLFW_KEY_F) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(window, true);  // Closes the application if the escape key is pressed
        }
        if (key == GLFW_KEY_LEFT_SHIFT)
            shiftPressed = true;

        if (key == GLFW_KEY_P && p_unpressed) {
            p_unpressed = false;

            g_chunkManager.serializeChunk(glm::ivec2(0, 0));
        }
        if (key == GLFW_KEY_O && o_unpressed) {
            o_unpressed = false;

            g_chunkManager.chunks.insert_or_assign(glm::ivec2(0, 0), g_chunkManager.deserializeChunk(glm::ivec2(0, 0)));
        }
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) shiftPressed = false;
        if (key == GLFW_KEY_P) p_unpressed = true;
        if (key == GLFW_KEY_O) o_unpressed = true;
    }
}

float g_cameraDistance = 5.0f;
float g_cameraAngleX = 0.0f;

float g_yaw = 0.0f;
float g_pitch = 0.0f;

// Scroll for zooming
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    if (shiftPressed) {
        g_cameraDistance -= yoffset * 0.1f;
        g_cameraDistance = std::max(g_cameraDistance, 0.1f);
    } else {
        g_yaw += xoffset * 0.04f;
        g_pitch += yoffset * 0.04f;

        float max = glm::pi<float>() / 2 - 0.01f;

        if (g_pitch > max) g_pitch = max;
        if (g_pitch < -max) g_pitch = -max;
    }
}

void errorCallback(int error, const char *desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
        1024, 768,
        "Interactive 3D Applications (OpenGL) - Simple Solar System", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
    glfwSetScrollCallback(g_window, scrollCallback);
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGL()) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK);                   // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE);                // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);                  // Specify the depth test for the z-buffer
    glEnable(GL_DEPTH_TEST);               // Enable the z-buffer test in the rasterization
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // specify the background color, used any time the framebuffer is cleared

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
}

void initGPUprogram() {
    g_program = glCreateProgram();  // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_program, GL_VERTEX_SHADER, "../resources/vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "../resources/fragmentShader.glsl");
    glLinkProgram(g_program);  // The main GPU program is ready to be handle streams of polygons

    // Cube map program
    g_cubeMapProgram = glCreateProgram();
    loadShader(g_cubeMapProgram, GL_VERTEX_SHADER, "../resources/cubeMapVertexShader.glsl");
    loadShader(g_cubeMapProgram, GL_FRAGMENT_SHADER, "../resources/cubeMapFragmentShader.glsl");
    glLinkProgram(g_cubeMapProgram);
}

void createCubeMap() {
    std::vector<float> vertexPositions = {
        // front
        -1.0f, -1.0f, 1.0f,  // 0
        1.0f, -1.0f, 1.0f,   // 1
        1.0f, 1.0f, 1.0f,    // 2
        -1.0f, 1.0f, 1.0f,   // 3
        // back
        -1.0f, -1.0f, -1.0f,  // 4
        1.0f, -1.0f, -1.0f,   // 5
        1.0f, 1.0f, -1.0f,    // 6
        -1.0f, 1.0f, -1.0f,   // 7
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

    g_starsCube = std::make_shared<Mesh>();
    g_starsCube->setGPUGeometry(posVbo, 0, 0, ibo, vao, numIndices);
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry() {
    createCubeMap();
    Chunk::init_chunks();
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(0.0, 0.0, 3.0));
    g_camera.setNear(0.1);
    g_camera.setFar(500);

    g_camera.setFoV(90);
}

void init() {
    initGLFW();
    initOpenGL();

    g_starsTexture = std::make_shared<Texture>("../resources/media/stars.jpg");

    initCPUgeometry();
    initGPUprogram();
    initCamera();
}

void clear() {
    glDeleteProgram(g_program);
    glDeleteProgram(g_cubeMapProgram);

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// The main rendering call
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Erase the color and z buffers.

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

    // Render stars
    glDepthMask(GL_FALSE);
    glUseProgram(g_cubeMapProgram);

    setUniform(g_cubeMapProgram, "u_viewMat", glm::mat4(glm::mat3(viewMatrix)));
    setUniform(g_cubeMapProgram, "u_projMat", projMatrix);

    setUniform(g_cubeMapProgram, "u_texture", 0);

    glActiveTexture(GL_TEXTURE0);
    g_starsTexture->bind();

    g_starsCube->render();

    glDepthMask(GL_TRUE);

    glUseProgram(g_program);

    setUniform(g_program, "u_viewMat", viewMatrix);
    setUniform(g_program, "u_projMat", projMatrix);

    setUniform(g_program, "u_cameraPosition", g_camera.getPosition());

    setUniform(g_program, "u_sunColor", glm::vec3(1.0f, 1.0f, 0.0f));
    setUniform(g_program, "u_sunPosition", glm::vec3(5.0f, 25.0f, 4.0f));

    setUniform(g_program, "u_ambientLight", glm::vec3(1.0f, 1.0f, 1.0f));

    setUniform(g_program, "u_texture", 0);
    setUniform(g_program, "u_objectColor", glm::vec3(1, 1, 1));

    g_chunkManager.renderAll(g_program);
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
    glm::vec3 targetPosition = glm::vec3(5.0f, 60.0f, 5.0f);
    g_camera.setTarget(targetPosition);

    // glm::vec3 cameraOffset = glm::normalize(glm::vec3(cos(g_cameraAngleX), 0.3f, sin(g_cameraAngleX))) * (1.1f + g_cameraDistance);
    glm::vec4 cameraOffset(0, 0, 1, 0);

    glm::mat4 rot1 = glm::rotate(glm::mat4(1), g_yaw, glm::vec3(0, 1, 0));
    glm::mat4 rot2 = glm::rotate(glm::mat4(1), g_pitch, glm::vec3(1, 0, 0));

    cameraOffset = g_cameraDistance * rot1 * rot2 * cameraOffset;

    g_camera.setPosition(targetPosition + glm::vec3(cameraOffset));

    g_chunkManager.updateQueue(targetPosition + glm::vec3(cameraOffset));
    g_chunkManager.generateOneChunk();
}

int main(int argc, char **argv) {
    init();
    while (!glfwWindowShouldClose(g_window)) {
        update(static_cast<float>(glfwGetTime()));
        render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
    }
    std::cout << std::endl;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}