#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vulkan/vulkan_core.h>

class TriangleWindow {
public:

    TriangleWindow(int width, int height, const char* name);
    ~TriangleWindow();
    
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    bool shouldClose() { return glfwWindowShouldClose(window); };
    void createSurface(VkInstance instance, VkSurfaceKHR* surface);
    void getFrameBufferSize(int* width, int* height) { return glfwGetFramebufferSize(window, width, height); };
    GLFWwindow* getWindow() { return window; };

private:
    const int width;
    const int height;

    const char* windowName;

    GLFWwindow* window;

    void initWindow();
    void initGlfwExtensions();
};