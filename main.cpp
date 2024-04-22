#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "chunk_manager.hpp"
#include "cube_map.hpp"

#include "gl_includes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "debug.hpp"

std::shared_ptr<CubeMap> g_cubeMap{};

// Window parameters
GLFWwindow *g_window{};

// GPU objects
GLuint g_program{};  // A GPU program contains at least a vertex shader and a fragment shader

Camera g_camera{};

ChunkManager g_chunkManager{};

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void window_size_callback(GLFWwindow *window, int width, int height) {
    g_camera.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height);  // Dimension of the rendering region in the window
}

bool shiftPressed = false;

// Executed each time a key is entered.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    g_camera.update_input_keys(key, action);

    static bool p_unpressed = false;
    static bool o_unpressed = false;
    static bool r_unpressed = false;
    static bool t_unpressed = false;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_Z) {
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
        if (key == GLFW_KEY_R && r_unpressed) {
            r_unpressed = false;

            g_chunkManager.reloadChunks();
        }
        if (key == GLFW_KEY_T && t_unpressed) {
            t_unpressed = false;

            g_chunkManager.saveChunks();
        }
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) shiftPressed = false;
        if (key == GLFW_KEY_P) p_unpressed = true;
        if (key == GLFW_KEY_O) o_unpressed = true;
        if (key == GLFW_KEY_R) r_unpressed = true;
        if (key == GLFW_KEY_T) t_unpressed = true;
    }
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    g_camera.update_input_mouse_pos(glm::vec2(xpos, ypos));
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    g_camera.update_input_mouse_button(button, action);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glm::ivec3 block_pos{};
        glm::vec3 dir = glm::normalize(g_camera.get_target() - g_camera.get_position());

        if (g_chunkManager.raycast(g_camera.get_position(), dir, 15, block_pos)) {
            std::cout << "Set block !! at {" << block_pos.x << ", " << block_pos.y << ", " << block_pos.z << "} ? :(\n";
            g_chunkManager.setBlock(block_pos, 0, true);
        } else {
            std::cout << "No block ? :(\n";
        }
    }
}

void initGLFW() {
    glfwSetErrorCallback(error_callback);

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
        "Minecraft clone attemps #93180289301", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, window_size_callback);
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    glfwSetCursorPosCallback(g_window, cursor_pos_callback);
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGL()) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
}

void initScene() {
    g_cubeMap = std::make_shared<CubeMap>();
    Chunk::init_chunks();

    // Init shader

    g_program = glCreateProgram();
    loadShader(g_program, GL_VERTEX_SHADER, "../resources/vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "../resources/fragmentShader.glsl");
    glLinkProgram(g_program);

    // Init camera

    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.set_near(0.1);
    g_camera.set_far(500);

    g_camera.set_fov(90);
}

void init() {
    initGLFW();
    initOpenGL();

    initScene();
}

void clear() {
    glDeleteProgram(g_program);

    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// The main rendering call
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Erase the color and z buffers.

    const glm::mat4 viewMatrix = g_camera.compute_view_matrix();
    const glm::mat4 projMatrix = g_camera.compute_projection_matrix();

    // Render stars
    g_cubeMap->render(projMatrix, viewMatrix);

    // Render the rest

    glUseProgram(g_program);

    setUniform(g_program, "u_viewMat", viewMatrix);
    setUniform(g_program, "u_projMat", projMatrix);

    setUniform(g_program, "u_cameraPosition", g_camera.get_position());

    setUniform(g_program, "u_sunColor", glm::vec3(1.0f, 1.0f, 0.0f));
    setUniform(g_program, "u_sunPosition", glm::vec3(5.0f, 25.0f, 4.0f));

    setUniform(g_program, "u_ambientLight", glm::vec3(1.0f, 1.0f, 1.0f));

    setUniform(g_program, "u_texture", 0);
    setUniform(g_program, "u_objectColor", glm::vec3(1, 1, 1));

    g_chunkManager.renderAll(g_program, g_camera);
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
    g_camera.update();

    glm::vec3 cam_pos = g_camera.get_position();

    g_chunkManager.updateQueue(cam_pos);
    g_chunkManager.generateOrLoadOneChunk(cam_pos);

    g_chunkManager.unloadUselessChunks(cam_pos);
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