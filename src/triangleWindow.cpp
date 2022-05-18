#include "triangleWindow.hpp"

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

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
}

TriangleWindow::~TriangleWindow()
{
    std::cout << "Deleting window...\n";
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