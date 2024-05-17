#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

// Basic camera model
class Camera {
   public:
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    bool mouse_locked = false;
    glm::vec3 m_pos = glm::vec3(8.0f, 70.0f, 8.0f);

    inline void set_position(const glm::vec3 &p) { m_pos = p; }
    inline glm::vec3 get_position() { return m_pos; }
    inline float get_fov() const { return m_fov; }
    inline void set_fov(const float f) { m_fov = f; }
    inline float get_aspect_ratio() const { return m_aspectRatio; }
    inline void set_aspect_ratio(const float a) { m_aspectRatio = a; }
    inline float get_near() const { return m_near; }
    inline void set_near(const float n) { m_near = n; }
    inline float get_far() const { return m_far; }
    inline void set_far(const float n) { m_far = n; }
    inline void set_target(const glm::vec3 &t) { m_target = t; }
    inline glm::vec3 get_target() { return m_target; }
    inline void set_screen_center(const glm::vec2 &p) { screen_center = p; }
    inline glm::vec2 get_screen_center() { return screen_center; }

    inline void init(int width, int height) {
        set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));

        set_near(0.1);
        set_far(500);

        set_fov(90);

        set_screen_center(glm::vec2(1024 / 2, 768 / 2));
    }

    /// @brief Compute the camera's view matrix based on internal parameters
    /// @return a view matrix
    inline glm::mat4 compute_view_matrix() const {
        return glm::lookAt(m_pos, m_target, glm::vec3(0, 1, 0));
    }

    /// @brief Compute the camera's projection matrix based on internal parameters
    /// @return a projection matrix
    inline glm::mat4 compute_projection_matrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
    }

    /// @brief Update the camera's pitch and yaw based on mouse position
    /// @param mouse_pos the mouse position
    void update_input_mouse_pos(GLFWwindow *window, glm::vec2 in_mouse_pos) {
        if (!mouse_locked) return;
        mouse_last_pos = mouse_pos;
        mouse_pos = in_mouse_pos;
        glm::vec2 mouse_delta_pos = mouse_pos - screen_center;

        glfwSetCursorPos(window, screen_center.x, screen_center.y);

        if (true || mouse_pressed) {
            m_yaw -= mouse_delta_pos.x * 0.005f;
            m_pitch += mouse_delta_pos.y * 0.005f;

            float max = glm::pi<float>() / 2 - 0.01f;

            if (m_pitch > max) m_pitch = max;
            if (m_pitch < -max) m_pitch = -max;
        }
    }

    /// @brief Update the mousepress state for left button
    /// @param button button gave by glfw callback
    /// @param action action gave by glfw callback
    void update_input_mouse_button(int button, int action) {
        if (mouse_locked) {
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (action == GLFW_PRESS) mouse_pressed = true;
                if (action == GLFW_RELEASE) mouse_pressed = false;
            }
        } else {
            if (action == GLFW_PRESS) mouse_locked = true;
        }
    }

    /// @brief Updates the camera position and target vector based on input and pitch and yaw
    void update(float delta_time) {
        // Adjust view

        glm::vec4 cameraOffset(0, 0, 1, 0);

        glm::mat4 rot1 = glm::rotate(glm::mat4(1), m_yaw, glm::vec3(0, 1, 0));
        glm::mat4 rot2 = glm::rotate(glm::mat4(1), m_pitch, glm::vec3(1, 0, 0));

        cameraOffset = rot1 * rot2 * cameraOffset;

        set_target(m_pos + glm::vec3(cameraOffset));
    }

   private:
    glm::vec3 m_target = glm::vec3(0, 0, 0);
    float m_fov = 90.f;         // Field of view, in degrees
    float m_aspectRatio = 1.f;  // Ratio between the width and the height of the image
    float m_near = 0.01f;       // Distance before which geometry is excluded from the rasterization process
    float m_far = 100.f;        // Distance after which the geometry is excluded from the rasterization process

    // Mouse

    bool mouse_pressed = false;
    glm::vec2 mouse_last_pos{};
    glm::vec2 mouse_pos{};
    glm::vec2 screen_center{};
};

#endif  // CAMERA_H