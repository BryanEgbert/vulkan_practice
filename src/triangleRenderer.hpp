#pragma once

#include "triangleDevice.hpp"
#include "triangleSwapchain.hpp"
#include "triangleUI.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <iostream>

namespace triangle
{
    class Renderer
    {
    public:
        
        Renderer(TriangleDevice& device, TriangleSwapchain& swapchain, TriangleUI& ui);
        ~Renderer();
        
        void beginCommandBuffer();
        void endCommandBuffer();

        void beginRenderPass();
        void endRenderpass();

        void submitBuffer();
        void destroyCommandBuffer();
    private:
        void createCommandBuffer();

        TriangleDevice& device;
        TriangleUI& ui;
        TriangleSwapchain swapchain{device};

        std::vector<vk::CommandBuffer> commandBuffers;

        uint32_t currentFrame = 0, imageIndex;
    };
}