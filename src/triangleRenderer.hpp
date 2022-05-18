#pragma once

#include "triangleDevice.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"
class TriangleRenderer
{
public:
    TriangleRenderer(TriangleDevice& device, TriangleSwapchain& swapchain, TrianglePipeline& pipeline);
    ~TriangleRenderer();

    void drawFrames();
private:

    TriangleDevice& device;
    TriangleSwapchain& swapchain;
    TrianglePipeline& graphicsPipeline;
};