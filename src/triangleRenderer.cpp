#include "triangleRenderer.hpp"
#include "triangleDevice.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"
#include "triangleECS.hpp"

#include <vulkan/vulkan.hpp>

#include <array>

namespace triangle
{
    Renderer::Renderer(TriangleDevice &device, TriangleWindow &window)
        : device{device}, window{window}
    {
        swapchain = std::make_unique<TriangleSwapchain>(device);

        commandBuffers.resize(swapchain->MAX_FRAMES_IN_FLIGHT);
        createCommandBuffer();
        createUICommandBuffer();

        setupDebugUI();
    }

    Renderer::~Renderer()
    {
        device.getLogicalDevice().destroyDescriptorPool(imguiDescPool);

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        destroyCommandBuffer();     
    }

    void Renderer::setupDebugUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        vk::DescriptorPoolSize poolSize[] =
        {
            vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000),
            vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 1000)
        };

        vk::DescriptorPoolCreateInfo poolCreateInfo(
            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            1000,
            std::size(poolSize),
            poolSize);

        imguiDescPool = device.getLogicalDevice().createDescriptorPool(poolCreateInfo);

        ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = static_cast<VkInstance>(device.getInstance());
        initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(device.getPhysicalDevice());
        initInfo.Device = static_cast<VkDevice>(device.getLogicalDevice());
        initInfo.Queue = static_cast<VkQueue>(device.getGraphicsQueue());
        initInfo.DescriptorPool = static_cast<VkDescriptorPool>(imguiDescPool);
        initInfo.MinImageCount = swapchain->getMinImageCount();
        initInfo.ImageCount = swapchain->getMinImageCount();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&initInfo, swapchain->getUIRenderPass());

        std::array<vk::CommandBuffer, 1> tempCommandBuffer;

        for (auto &commandBuffer : tempCommandBuffer)
        {
            device.beginSingleTimeCommands(commandBuffer);
            ImGui_ImplVulkan_CreateFontsTexture(static_cast<VkCommandBuffer>(commandBuffer));
            device.endSingleTimeCommand(commandBuffer);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void Renderer::createCommandBuffer()
    {

        vk::CommandBufferAllocateInfo cmdBufferAllocateInfo(
            device.getCommandPool(),
            vk::CommandBufferLevel::ePrimary,
            commandBuffers.size());

        commandBuffers = device.getLogicalDevice().allocateCommandBuffers(cmdBufferAllocateInfo);
    }

    void Renderer::createUICommandBuffer()
    {
        uiCommandBuffers.resize(swapchain->MAX_FRAMES_IN_FLIGHT);

        vk::CommandBufferAllocateInfo allocInfo(
            device.getCommandPool(),
            vk::CommandBufferLevel::ePrimary,
            uiCommandBuffers.size());

        uiCommandBuffers = device.getLogicalDevice().allocateCommandBuffers(allocInfo);
    }

    void Renderer::createUI(std::function<void()> frameCallback)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        frameCallback();
    }

    vk::CommandBuffer Renderer::beginCommandBuffer()
    {
        vk::Result result = swapchain->acquireNextImage(&imageIndex, currentFrame);

        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            recreateSwapchain();
            return nullptr;
        }

        renderUI();

        commandBuffers[currentFrame].reset();
        vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlags(), nullptr);

        commandBuffers[currentFrame].begin(commandBufferBeginInfo);

        return commandBuffers[currentFrame];
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
            swapchain->getMainRenderPass(),
            swapchain->getMainFramebuffers()[imageIndex],
            {{0, 0}, swapchain->getExtent()},
            clearValues);
        
        commandBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(swapchain->getExtent().width), static_cast<float>(swapchain->getExtent().height), 0.0f, 1.0f);
        commandBuffers[currentFrame].setViewport(0, viewport);

        vk::Rect2D scissor({0, 0}, swapchain->getExtent());
        commandBuffers[currentFrame].setScissor(0, scissor);
    }

    void Renderer::endRenderpass()
    {
        commandBuffers[currentFrame].endRenderPass();
    }

    void Renderer::submitBuffer()
    {
        device.getLogicalDevice().resetFences(swapchain->getInFlightFences(currentFrame));

        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        std::array<vk::Semaphore, 1> waitSemaphore = {swapchain->getPresentSemaphore(currentFrame)};
        std::array<vk::Semaphore, 1> signalSemaphore = {swapchain->getRenderSemaphore(currentFrame)};

        std::array<vk::CommandBuffer, 2> currentCommandBuffer = {commandBuffers[currentFrame], uiCommandBuffers[currentFrame]};

        vk::SubmitInfo submitInfo(waitSemaphore, waitStages, currentCommandBuffer, signalSemaphore);
        std::vector<vk::SubmitInfo> submitInfos = {submitInfo};

        device.getGraphicsQueue().submit(submitInfos, swapchain->getInFlightFences(currentFrame));

        std::array<vk::SwapchainKHR, 1> swapchains = {swapchain->getSwapchain()};

        vk::Result result;
        vk::PresentInfoKHR presentInfo(signalSemaphore, swapchains, imageIndex, result);

        result = device.getPresentQueue().presentKHR(presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.isWindowResized())
        {
            window.resetWindowResizeFlag();
        }

        currentFrame = (currentFrame + 1) % swapchain->MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::recreateSwapchain()
    {
        auto extent = swapchain->getExtent();
        while (extent.width == 0 && extent.height == 0)
        {
            extent = swapchain->getExtent();
            glfwWaitEvents();
        }

        device.getLogicalDevice().waitIdle();
        swapchain = std::make_unique<TriangleSwapchain>(device);
    }

    void Renderer::destroyCommandBuffer()
    {
        device.getLogicalDevice().freeCommandBuffers(device.getCommandPool(), commandBuffers);
    }

    void Renderer::renderUI()
    {
        ImGui::Render();
        uiCommandBuffers[currentFrame].reset();
        // No need to reset command pool because the ui command buffer is using the same command pool
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        uiCommandBuffers[currentFrame].begin(beginInfo);

        std::array<vk::ClearValue, 1> clearValues;
        clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>({{0.0f, 0.0f, 0.0f, 1.0f}})));

        vk::RenderPassBeginInfo renderPassBeginInfo(
            swapchain->getUIRenderPass(),
            swapchain->getUIFramebuffers()[imageIndex],
            {{0, 0}, swapchain->getExtent()},
            clearValues);

        uiCommandBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), uiCommandBuffers[currentFrame]);

        uiCommandBuffers[currentFrame].endRenderPass();
        uiCommandBuffers[currentFrame].end();
    }
}
