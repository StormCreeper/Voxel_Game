#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "camera.hpp"
#include "utils/gl_includes.hpp"

class Player {
   private:
    // Keys
    bool front_pressed{};
    bool back_pressed{};
    bool left_pressed{};
    bool right_pressed{};
    bool up_pressed{};
    bool down_pressed{};

    bool speed_pressed{};

   public:
    Camera m_camera{};

    /// @brief Updates the camera position and target vector based on input and pitch and yaw
    void update(float delta_time) {
        // Adjust camera position based on input keys (WASD)
        float camera_speed = 30.f * delta_time;
        if (speed_pressed) camera_speed *= 4.f;
        glm::vec3 front;
        glm::vec3 right;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Compute camera front and right vectors based on yaw
        glm::vec3 front_tmp;
        front_tmp.x = sin(m_camera.m_yaw);
        front_tmp.y = 0.0f;
        front_tmp.z = cos(m_camera.m_yaw);
        front = glm::normalize(front_tmp);
        right = glm::normalize(glm::cross(front, up));

        // Update camera position
        if (front_pressed)
            m_camera.m_pos += front * camera_speed;
        if (back_pressed)
            m_camera.m_pos -= front * camera_speed;
        if (left_pressed)
            m_camera.m_pos -= right * camera_speed;
        if (right_pressed)
            m_camera.m_pos += right * camera_speed;
        if (down_pressed)
            m_camera.m_pos -= up * camera_speed;
        if (up_pressed)
            m_camera.m_pos += up * camera_speed;

        m_camera.update(delta_time);
    }

    /// @brief Update the keypress states for wasd, space and left ctrl
    void update_input_keys(int key, int action) {
        if (key == GLFW_KEY_W) front_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_S) back_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_A) left_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_D) right_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_SPACE) up_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_LEFT_CONTROL) down_pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_LEFT_SHIFT) speed_pressed = action != GLFW_RELEASE;

        if (m_camera.mouse_locked && key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            m_camera.mouse_locked = false;
        }
    }
};

#endif  // PLAYER_HPP