#include "triangleWindow.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace triangle
{
    Window::Window(int width, int height, const char* name) 
        : width{width}, height{height}, windowName{name}
    {
        initWindow();
        initGlfwExtensions();
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::initWindow()
    {
        glfwInit();

        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[1]);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(mode->width, mode->height, windowName, monitors[1], nullptr);
        
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

        glfwSetWindowMonitor(window, monitors[1], 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    void Window::initGlfwExtensions()
    {
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    }

    void Window::frameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto triangleWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        triangleWindow->isFrameBufferResized = true; 
        triangleWindow->width = width;
        triangleWindow->height = height;
    }

    void Window::createSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance,  window, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface");
    }
}