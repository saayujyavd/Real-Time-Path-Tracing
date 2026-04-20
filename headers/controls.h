#ifndef CONTROLS_H
#define CONTROLS_H

#include <cstdio>
#include "camera.h"

#define PI 3.14159265f
#define radians(x) ((x) * PI / 180.0f)
#define degrees(x) ((x) * 180.0f / PI)

float cam_speed = 0.5f;
float last_x, last_y;
bool  first_move = true;

vec3 init_dir = unit_vector(point3(0, 0, 0) - point3(13, 2, 3));
float cam_pitch = degrees(asinf(init_dir.y()));
float cam_yaw = degrees(atan2f(init_dir.z(), init_dir.x()));

void mouseCB(GLFWwindow* window, double xpos, double ypos)
{
    // 1. INPUT GATE: Only process rotation if Right Mouse is held
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
        first_move = true; // Reset so we don't "jump" when we click again
        return;
    }

    Camera* cam = (Camera*)glfwGetWindowUserPointer(window);

    // 2. INITIALIZATION: Set starting position on the first frame of a click
    if (first_move) {
        last_x = (float)xpos;
        last_y = (float)ypos;
        first_move = false;
        return;
    }

    // 3. DELTA CALCULATION: How far has the mouse moved since last frame?
    float dx = ((float)xpos - last_x) * 0.1f;
    float dy = (last_y - (float)ypos) * 0.1f; // Y is inverted in screen space
    last_x = (float)xpos;
    last_y = (float)ypos;

    // 4. ANGLE UPDATE: Apply movement to Euler angles
    cam_yaw += dx;
    cam_pitch += dy;

    // 5. CONSTRAINTS: Prevent the camera from flipping over at the poles
    if (cam_pitch > 89.0f) cam_pitch = 89.0f;
    if (cam_pitch < -89.0f) cam_pitch = -89.0f;

    // 6. DIRECTION MATH: Convert Yaw/Pitch into a 3D vector
    vec3 dir;
    dir[0] = cos(radians(cam_yaw)) * cos(radians(cam_pitch));
    dir[1] = sin(radians(cam_pitch));
    dir[2] = sin(radians(cam_yaw)) * cos(radians(cam_pitch));

    // 7. APPLY: Update the camera's target point
    cam->lookat = cam->lookfrom + unit_vector(dir);
    cam->update();
}

void processKeys(GLFWwindow* window, Camera* cam)
{
    // --- CURSOR STATE ---
    // Hide and lock cursor only when interacting with the world
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // --- MOVEMENT MATH ---
    // Calculate relative "Forward" and "Right" directions
    vec3 current_dir = unit_vector(cam->lookat - cam->lookfrom);
    vec3 right = unit_vector(cross(current_dir, vec3(0, 1, 0)));

    // --- KEY HANDLING ---
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam->lookfrom = cam->lookfrom + cam_speed * current_dir;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam->lookfrom = cam->lookfrom - cam_speed * current_dir;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam->lookfrom = cam->lookfrom - cam_speed * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam->lookfrom = cam->lookfrom + cam_speed * right;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Recalculate lookat so the view direction stays fixed as the origin moves
    cam->lookat = cam->lookfrom + current_dir;
    cam->update();
}

#endif
