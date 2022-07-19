#include "triangleCamera.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <math.h>

TriangleCamera::TriangleCamera(GLFWwindow* window, const int windowWidth, const int windowHeight) 
    : window{window}, windowWidth{windowWidth}, windowHeight{windowHeight}
{
}

void TriangleCamera::setCamera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp)
{
    CameraProperties.cameraPos = cameraPos;
    CameraProperties.cameraFront = cameraFront;
    CameraProperties.cameraUp = cameraUp;
}

void TriangleCamera::processCameraMovement()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float camSpeed = 6.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        CameraProperties.cameraPos += camSpeed * CameraProperties.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        CameraProperties.cameraPos -= camSpeed * CameraProperties.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        CameraProperties.cameraPos -= glm::normalize(glm::cross(CameraProperties.cameraFront, CameraProperties.cameraUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        CameraProperties.cameraPos += glm::normalize(glm::cross(CameraProperties.cameraFront, CameraProperties.cameraUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        CameraProperties.cameraPos.y += camSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        CameraProperties.cameraPos.y -= camSpeed;
}

void TriangleCamera::processCameraRotation(const float sensitivity)
{
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    CameraProperties.cameraFront = glm::normalize(direction);
}

