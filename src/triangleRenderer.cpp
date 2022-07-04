#include "triangleRenderer.hpp"
#include "triangleDevice.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"
#include "triangleECS.hpp"

#include <vulkan/vulkan.hpp>

#include <array>
#include <cstdint>

namespace triangle
{
    Renderer::Renderer(TriangleDevice &device, TriangleSwapchain &swapchain, TriangleUI &ui)
        : device{device}, swapchain{swapchain}, ui{ui}
    {
        createCommandBuffer();
    }

    Renderer::~Renderer()
    {
        destroyCommandBuffer();        
    }

    void Renderer::createCommandBuffer()
    {
        commandBuffers.resize(swapchain.MAX_FRAMES_IN_FLIGHT);

        vk::CommandBufferAllocateInfo cmdBufferAllocateInfo(
            device.getCommandPool(),
            vk::CommandBufferLevel::ePrimary,
            commandBuffers.size());

        commandBuffers = device.getLogicalDevice().allocateCommandBuffers(cmdBufferAllocateInfo);
    }

    void Renderer::beginCommandBuffer()
    {
        swapchain.acquireNextImage(&imageIndex, currentFrame);

        ui.renderFrame(imageIndex, currentFrame);

        vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlags(), nullptr);
        commandBuffers[currentFrame].begin(commandBufferBeginInfo);
    }

    void Renderer::endCommandBuffer()
    {
        commandBuffers[currentFrame].end();
    }

    void Renderer::beginRenderPass()
    {
        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>({{0.0f, 0.0f, 0.0f, 1.0f}})));
        clearValues[1].setDepthStencil({1.0f, 0});

        vk::RenderPassBeginInfo renderPassBeginInfo(
            swapchain.getMainRenderPass(),
            swapchain.getMainFramebuffers()[imageIndex],
            {{0, 0}, swapchain.getExtent()},
            clearValues);
        
        commandBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    }

    void Renderer::endRenderpass()
    {
        commandBuffers[currentFrame].endRenderPass();
    }

    void Renderer::submitBuffer()
    {
        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        std::array<vk::Semaphore, 1> waitSemaphore = {swapchain.getPresentSemaphore(currentFrame)};
        std::array<vk::Semaphore, 1> signalSemaphore = {swapchain.getRenderSemaphore(currentFrame)};

        std::array<vk::CommandBuffer, 2> currentCommandBuffer = {commandBuffers[currentFrame], ui.getCommandBuffers(currentFrame)};
    }

    void Renderer::destroyCommandBuffer()
    {
        device.getLogicalDevice().freeCommandBuffers(device.getCommandPool(), commandBuffers);
    }
}
