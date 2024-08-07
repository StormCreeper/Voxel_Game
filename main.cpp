#include "gl_objects/mesh.hpp"
#include "player.hpp"
#include "gl_objects/shader.hpp"
#include "gl_objects/texture.hpp"
#include "chunks/chunk_manager.hpp"
#include "chunks/chunk_dealer.hpp"
#include "cube_map.hpp"

#include "utils/gl_includes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "utils/debug.hpp"

std::shared_ptr<CubeMap> g_cubeMap{};

// Window parameters
GLFWwindow *g_window{};

// GPU objects
GLuint g_program{};  // A GPU program contains at least a vertex shader and a fragment shader

Player g_player{};

ChunkManager *g_chunkManager{};
ChunkDealer *g_chunkDealer{};

glm::mat4 g_viewMatrix;
glm::mat4 g_projMatrix;

int g_tool = 1;

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void window_size_callback(GLFWwindow *window, int width, int height) {
    g_player.m_camera.set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height);  // Dimension of the rendering region in the window

    g_player.m_camera.set_screen_center(glm::vec2(width / 2, height / 2));

    g_projMatrix = g_player.m_camera.compute_projection_matrix();
}

bool shiftPressed = false;

// Executed each time a key is entered.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    g_player.update_input_keys(key, action);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_Z) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (key == GLFW_KEY_F) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if ((key == GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(window, true);  // Closes the application if the escape key is pressed
        }
        if (key == GLFW_KEY_LEFT_SHIFT)
            shiftPressed = true;

        if (key == GLFW_KEY_R) {
            // g_chunkManager->reloadChunks();
        }
        if (key == GLFW_KEY_T) {
            g_chunkManager->saveChunks();
        }
        if (key == GLFW_KEY_LEFT) {
            int size = BlockPalette::block_descs.size();
            if (--g_tool <= 0) g_tool = size - 1;
        }
        if (key == GLFW_KEY_RIGHT) {
            int size = BlockPalette::block_descs.size();
            if (++g_tool >= size) g_tool = 1;
        }
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) shiftPressed = false;
    }
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    g_player.m_camera.update_input_mouse_pos(window, glm::vec2(xpos, ypos));
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    g_player.m_camera.update_input_mouse_button(button, action);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glm::ivec3 block_pos{};
        glm::ivec3 normal{};
        glm::vec3 dir = glm::normalize(g_player.m_camera.get_target() - g_player.m_camera.get_position());

        if (g_chunkManager->raycast(g_player.m_camera.get_position(), dir, 15, block_pos, normal)) {
            std::cout << "Remove block !! at {" << block_pos.x << ", " << block_pos.y << ", " << block_pos.z << "} ? :(\n";
            g_chunkManager->setBlock(block_pos, 0, true);
        } else {
            std::cout << "No block ? :(\n";
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glm::ivec3 block_pos{};
        glm::ivec3 normal{};
        glm::vec3 dir = glm::normalize(g_player.m_camera.get_target() - g_player.m_camera.get_position());

        if (g_chunkManager->raycast(g_player.m_camera.get_position(), dir, 15, block_pos, normal)) {
            std::cout << "Set block !! at {" << block_pos.x << ", " << block_pos.y << ", " << block_pos.z << "} ? Block ID: " << g_tool << " :(\n";
            g_chunkManager->setBlock(block_pos + normal, g_tool, true);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

    // glfwSwapInterval(0);
}

void initScene() {
    g_cubeMap = std::make_shared<CubeMap>();
    Chunk::init_chunks();

    g_chunkManager = new ChunkManager();
    g_chunkDealer = new ChunkDealer(100, g_chunkManager);
    g_chunkManager->chunk_dealer = g_chunkDealer;

    // Init shader

    g_program = glCreateProgram();
    loadShader(g_program, GL_VERTEX_SHADER, "../resources/vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "../resources/fragmentShader.glsl");
    glLinkProgram(g_program);

    // Init camera

    int width, height;
    glfwGetWindowSize(g_window, &width, &height);

    g_player.m_camera.init(width, height);
}

float last_time = 0;
int nb_frames = 0;

void init() {
    initGLFW();
    initOpenGL();

    initScene();

    last_time = glfwGetTime();

    BlockPalette::bind_texture(g_program);

    glUseProgram(g_program);

    setUniform(g_program, "u_texture", 0);

    setUniform(g_program, "u_chunkSize", Chunk::chunk_size);

    g_projMatrix = g_player.m_camera.compute_projection_matrix();
}

void render() {
    nb_frames++;
    float time_now = glfwGetTime();
    if (time_now - last_time > 1) {
        float fps = nb_frames / (time_now - last_time);

        std::stringstream ss;
        ss << "Minecraft clone attemps #93180289301 - " << fps << " FPS";

        glfwSetWindowTitle(g_window, ss.str().c_str());
        nb_frames = 0;
        last_time = time_now;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_viewMatrix = g_player.m_camera.compute_view_matrix();

    // Render stars

    g_cubeMap->render(g_projMatrix, g_viewMatrix);

    // Render the rest

    glUseProgram(g_program);

    setUniform(g_program, "u_viewProjMat", g_projMatrix * g_viewMatrix);

    g_chunkManager->renderAll(g_program, g_player.m_camera);
}

float last_physic_time = 0;

void update(const float currentTimeInSec) {
    float delta_time = currentTimeInSec - last_physic_time;
    last_physic_time = currentTimeInSec;

    g_player.update(delta_time);

    glm::vec3 cam_pos = g_player.m_camera.get_position();

    g_chunkManager->updateQueue(cam_pos);

    g_chunkManager->unloadUselessChunks();
}

void clear() {
    g_chunkManager->destroy();
    delete g_chunkManager;
    delete g_chunkDealer;

    glDeleteProgram(g_program);

    glfwDestroyWindow(g_window);
    glfwTerminate();
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