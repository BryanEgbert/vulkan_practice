#pragma once

#include "triangleDevice.hpp"
#include "triangleWindow.hpp"

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>

namespace triangle
{

    class Swapchain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        struct Texture
        {
            vk::Sampler sampler;
            vk::ImageView imageView;
            vk::ImageLayout imageLayout;
            vk::Image image;
            vk::DeviceMemory deviceMemory;
            uint32_t width, height;
            uint32_t mipLevels;
        } textureProperties;

        Swapchain(Device& device);
        ~Swapchain();

        vk::Extent2D getExtent() { return swapchainExtent; };
        vk::SurfaceFormatKHR getFormat() { return format; };
        vk::RenderPass getMainRenderPass() { return mainRenderPass; };
        vk::RenderPass getUIRenderPass() { return uiRenderPass; };
        std::vector<vk::Framebuffer> getMainFramebuffers() { return mainFrameBuffers; };
        std::vector<vk::Framebuffer> getUIFramebuffers() { return uiFrameBuffers; };
        vk::SwapchainKHR getSwapchain() { return swapchain; };

        vk::Result acquireNextImage(uint32_t* imageIndex, uint32_t& currentFrame);
        vk::Fence getInFlightFences(uint32_t index) { return inFlightFences[index]; };

        vk::Semaphore getRenderSemaphore(int index) { return renderFinishedSemaphore[index]; };
        vk::Semaphore getPresentSemaphore(int index) { return imageAvailableSemaphore[index]; };
        uint32_t getMinImageCount() { return swapChainSupportDetails.capabilities.minImageCount + 1; };

    private:
        Device& device;
        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        struct SwapChainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        } swapChainSupportDetails;

        vk::SurfaceFormatKHR format;
        vk::PresentModeKHR presentMode;
        vk::Extent2D swapchainExtent;

        vk::SwapchainKHR swapchain;
        std::vector<vk::ImageView> imageViews;
        std::vector<vk::Framebuffer> mainFrameBuffers, uiFrameBuffers;

        vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;

        vk::RenderPass mainRenderPass, uiRenderPass;
        
        std::vector<vk::Semaphore> imageAvailableSemaphore, renderFinishedSemaphore;
        std::vector<vk::Fence> inFlightFences;

        void querySwapchainSupport();
        void initSurfaceProperties();
        void createSwapchain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createUIRenderPass();
        void createFrameBuffers();
        void createSyncObject();
        // void loadTextureFromBuffer
        void loadTextureFromFile(const std::string& filename);

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
        vk::Format findDepthFormat();
    };
}