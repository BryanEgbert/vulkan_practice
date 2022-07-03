#pragma once

#include "triangleDevice.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"

#include <vulkan/vulkan.hpp>

class TriangleRenderer
{
public:
    TriangleRenderer(TriangleDevice& device, TriangleSwapchain& swapchain);
    ~TriangleRenderer();

    void drawFrames();
private:
    void drawUI();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffer();

    TriangleDevice& device;
    TriangleSwapchain& swapchain;

    vk::PipelineLayout pipelineLayout;
    std::unique_ptr<TrianglePipeline> trianglePipeline;
    std::unique_ptr<TriangleModel> triangleModel;
    // std::unique_ptr<TriangleDescriptor> triangleDescriptor;
    // std::unique_ptr<TriangleCamera> triangleCamera;
    std::vector<vk::CommandBuffer> commandBuffers;
};