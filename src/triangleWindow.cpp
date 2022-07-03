#include "triangleWindow.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

TriangleWindow::TriangleWindow(int width, int height, const char* name) 
    : width{width}, height{height}, windowName{name}
{
    initWindow();
    initGlfwExtensions();
}

void TriangleWindow::initWindow()
{
    glfwInit();

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName, monitors[0], nullptr);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

TriangleWindow::~TriangleWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
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