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
    void initUI();
private:
    TriangleDevice& device;
    TriangleWindow& window;
    TriangleSwapchain&  swapchain;

    vk::DescriptorPool imguiPool;
    vk::CommandBuffer uiCommandBuffer;
    
    void createUIDescriptorPool();
};