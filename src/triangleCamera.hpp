#pragma once

#include "triangleWindow.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <glm/gtc/quaternion.hpp>
#include <vulkan/vulkan.hpp>

class TriangleCamera
{
public:
    TriangleCamera(GLFWwindow* window, const int windowWidth, const int windowHeight);

    void setCamera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp = {0.0f, 1.0f, 0.0f});
    glm::mat4 getView() { return view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); };
    // glm::mat4 getQuatView() { return view = glm::mat4_cast(glm::quatLookAt(cameraPos - cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));};

    void processCameraMovement();
    void processCameraRotation(const float sensitivity);
private:
    float deltaTime = 0.0f, lastFrame = 0.0f;

    GLFWwindow* window;
    int windowWidth, windowHeight;

    bool firstMouse = true;
    double lastX = 400, lastY = 300;
    double xpos, ypos;

    glm::vec3 cameraPos, cameraFront, cameraUp, direction;
    float yaw, pitch;

    glm::mat4 view, proj;
};