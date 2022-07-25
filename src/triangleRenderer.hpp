#pragma once

#include "triangleDevice.hpp"
#include "triangleSwapchain.hpp"

#include <vulkan/vulkan.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui.h>

#include <memory>
#include <functional>
#include <iostream>

namespace triangle
{
    class Renderer
    {
    public:
        
        Renderer(Device& device, Window& window);
        ~Renderer();
        
        vk::CommandBuffer& getCurrentCommandBuffer() { return commandBuffers.at(currentFrame); }
        uint32_t getCurrentFrame() { return currentFrame; }
        int getMaxFramesInFlight() { return swapchain->MAX_FRAMES_IN_FLIGHT; }
        vk::RenderPass getMainRenderPass() { return swapchain->getMainRenderPass(); }
        float getAspectRatio() { return swapchain->getExtent().width / swapchain->getExtent().height; }
        ImGuiIO& getUiIO() { return ImGui::GetIO(); }
        Swapchain::Texture getTextureProperties() { return swapchain->textureProperties; }

        void createUI(std::function<void()> frameCallback);

        vk::CommandBuffer beginCommandBuffer();
        void endCommandBuffer();

        void beginRenderPass();
        void endRenderpass();

        void submitBuffer();
        void destroyCommandBuffer();
    private:
        void createCommandBuffer();
        void createUICommandBuffer();
        void recreateSwapchain();

        void setupDebugUI();
        void renderUI();

        Device& device;
        std::unique_ptr<Swapchain> swapchain;
        Window& window;

        std::vector<vk::CommandBuffer> commandBuffers, uiCommandBuffers;
        vk::DescriptorPool imguiDescPool;

        uint32_t currentFrame = 0, imageIndex;
    };
}