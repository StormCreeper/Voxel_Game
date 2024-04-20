#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

// Basic camera model
class Camera {
   public:
    inline float get_fov() const { return m_fov; }
    inline void set_fov(const float f) { m_fov = f; }
    inline float get_aspect_ratio() const { return m_aspectRatio; }
    inline void set_aspect_ratio(const float a) { m_aspectRatio = a; }
    inline float get_near() const { return m_near; }
    inline void set_near(const float n) { m_near = n; }
    inline float get_far() const { return m_far; }
    inline void set_far(const float n) { m_far = n; }
    inline void set_position(const glm::vec3 &p) { m_pos = p; }
    inline glm::vec3 get_position() { return m_pos; }
    inline void set_target(const glm::vec3 &t) { m_target = t; }
    inline glm::vec3 get_target() { return m_target; }

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
    void update_input_mouse_pos(glm::vec2 in_mouse_pos) {
        mouse_last_pos = mouse_pos;
        mouse_pos = in_mouse_pos;
        glm::vec2 mouse_delta_pos = mouse_pos - mouse_last_pos;
        if (mouse_pressed) {
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
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) mouse_pressed = true;
            if (action == GLFW_RELEASE) mouse_pressed = false;
        }
    }
    /// @brief Update the keypress states for wasd, space and left ctrl
    void update_input_keys(int key, int action) {
        if (key == GLFW_KEY_W) front_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_S) back_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_A) left_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_D) right_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_SPACE) up_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_LEFT_CONTROL) down_pressed = action != GLFW_RELEASE;
    }

    /// @brief Updates the camera position and target vector based on input and pitch and yaw
    void update() {
        // Adjust camera position based on input keys (WASD)
        const float camera_speed = 0.5f;
        glm::vec3 front;
        glm::vec3 right;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Compute camera front and right vectors based on yaw
        glm::vec3 front_tmp;
        front_tmp.x = sin(m_yaw);
        front_tmp.y = 0.0f;
        front_tmp.z = cos(m_yaw);
        front = glm::normalize(front_tmp);
        right = glm::normalize(glm::cross(front, up));

        // Update camera position
        if (front_pressed)
            m_pos += front * camera_speed;
        if (back_pressed)
            m_pos -= front * camera_speed;
        if (left_pressed)
            m_pos -= right * camera_speed;
        if (right_pressed)
            m_pos += right * camera_speed;
        if (down_pressed)
            m_pos -= up * camera_speed;
        if (up_pressed)
            m_pos += up * camera_speed;

        // Adjust view

        glm::vec4 cameraOffset(0, 0, 1, 0);

        glm::mat4 rot1 = glm::rotate(glm::mat4(1), m_yaw, glm::vec3(0, 1, 0));
        glm::mat4 rot2 = glm::rotate(glm::mat4(1), m_pitch, glm::vec3(1, 0, 0));

        cameraOffset = rot1 * rot2 * cameraOffset;

        set_target(m_pos + glm::vec3(cameraOffset));
    }

   private:
    glm::vec3 m_pos = glm::vec3(8.0f, 70.0f, 8.0f);
    glm::vec3 m_target = glm::vec3(0, 0, 0);
    float m_fov = 90.f;         // Field of view, in degrees
    float m_aspectRatio = 1.f;  // Ratio between the width and the height of the image
    float m_near = 0.01f;       // Distance before which geometry is excluded from the rasterization process
    float m_far = 100.f;        // Distance after which the geometry is excluded from the rasterization process

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    // Mouse

    bool mouse_pressed = false;
    glm::vec2 mouse_last_pos{};
    glm::vec2 mouse_pos{};

    // Keys
    bool front_pressed{};
    bool back_pressed{};
    bool left_pressed{};
    bool right_pressed{};
    bool up_pressed{};
    bool down_pressed{};
};

#endif  // CAMERA_H