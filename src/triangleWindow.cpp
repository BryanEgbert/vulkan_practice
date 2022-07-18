#include "triangleWindow.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

TriangleWindow::TriangleWindow(int width, int height, const char* name) 
    : width{width}, height{height}, windowName{name}
{
    initWindow();
    initGlfwExtensions();
}

TriangleWindow::~TriangleWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void TriangleWindow::initWindow()
{
    glfwInit();

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetWindowUserPointer(window, this);
}

void TriangleWindow::initGlfwExtensions()
{
    std::cout << "GLFW version: " << glfwGetVersionString() << '\n';
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::cout << "GLFW Extensions:\n";
    for (int i = 0; i < glfwExtensionCount; ++i)
        std::cout << glfwExtensions[i] << '\n';
}

void TriangleWindow::createSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance,  window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}