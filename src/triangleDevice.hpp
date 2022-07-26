#pragma once

#include "triangleWindow.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace triangle
{
    class Device
    {
    public:
        Device(const char* appName, Window& window);
        ~Device();

        vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; };
        vk::Device getLogicalDevice() { return device; };
        vk::SurfaceKHR getSurface() { return static_cast<vk::SurfaceKHR>(surface); };
        auto getQueueFamilyIndex() { return queueFamilyIndex; };
        vk::CommandPool getCommandPool() { return mainCommandPool; };

        vk::Queue getGraphicsQueue() { return graphicsQueue; };
        vk::Queue getPresentQueue() { return presentQueue; };

        vk::Instance getInstance() { return instance; };

        void beginSingleTimeCommands(vk::CommandBuffer& cmdBuffer);
        void endSingleTimeCommand(vk::CommandBuffer& cmdBuffer);

        void copyBuffer(vk::Buffer& srcBuffer, vk::Buffer& dstBuffer, vk::DeviceSize size);
        void createBuffer(  vk::DeviceSize size, 
                            vk::BufferUsageFlags usage, 
                            vk::MemoryPropertyFlags properties, 
                            vk::Buffer&  buffer, 
                            vk::DeviceMemory& bufferMemory);
        void allocateAndBindImage(vk::DeviceMemory& imageMemory, vk::Image& image, vk::MemoryPropertyFlags properties);
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    private:
        struct SwapChainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        }swapChainSupportDetails;

        const char* appName;
        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        Window& window;

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugUtilsMessenger;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::CommandPool mainCommandPool;

        vk::PhysicalDeviceMemoryProperties memProperties;
        vk::MemoryRequirements memRequirements;


        struct QueueFamilyIndex
        {
            uint32_t graphics, present;
        }queueFamilyIndex;

        VkSurfaceKHR surface;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

        void createInstance();
        void createSurface();
        void createDevice();
        void createCommandPool();

        void createDebugMessenger(vk::DebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo);
        static bool checkRequiredLayers(const std::vector<const char*>& instanceLayers);
        void querySwapchainSupport();

        #ifdef NDEBUG
            const bool enableValidationLayers = false;
        #else
            const bool enableValidationLayers = true;
        #endif
    };
}