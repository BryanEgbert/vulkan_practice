#pragma once

#include "triangleDevice.hpp"
#include "triangleSwapchain.hpp"
#include "triangleWindow.hpp"
#include "vulkan/vulkan_handles.hpp"
#include <vulkan/vulkan.hpp>

class TriangleUI
{
public:
    TriangleUI(TriangleDevice& device, TriangleWindow& window, TriangleSwapchain& swapchain);
    ~TriangleUI();

    vk::CommandBuffer getCommandBuffers(int index) { return uiCommandBuffers[index]; };

    void beginUIFrame();
    void draw();
    void renderFrame(uint32_t& imageIndex, uint32_t& currentFrame);
private:
    TriangleDevice& device;
    TriangleWindow& window;
    TriangleSwapchain&  swapchain;

    vk::DescriptorPool imguiPool;
    
    std::vector<vk::CommandBuffer> uiCommandBuffers;
    
    void createUIDescriptorPool();
    void createUICommandBuffer();
};