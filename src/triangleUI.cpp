#include "triangleUI.hpp"
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui.h>
#include "triangleDevice.hpp"
#include "triangleSwapchain.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"


TriangleUI::TriangleUI(TriangleDevice& device, TriangleWindow& window, TriangleSwapchain& swapchain) 
    : device{device}, window{window}, swapchain{swapchain}
{
    std::cout << "Create UI\n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    createUIDescriptorPool();
    createUICommandBuffer();
}

TriangleUI::~TriangleUI()
{
    device.getLogicalDevice().destroyDescriptorPool(imguiPool);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void TriangleUI::createUICommandBuffer()
{
    uiCommandBuffers.resize(swapchain.MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocInfo(
        device.getCommandPool(),
        vk::CommandBufferLevel::ePrimary,
        uiCommandBuffers.size()
    );

    uiCommandBuffers = device.getLogicalDevice().allocateCommandBuffers(allocInfo);
}

void TriangleUI::createUIDescriptorPool()
{
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
        poolSize
    );

    imguiPool = device.getLogicalDevice().createDescriptorPool(poolCreateInfo);

    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = static_cast<VkInstance>(device.getInstance());
    initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(device.getPhysicalDevice());
    initInfo.Device = static_cast<VkDevice>(device.getLogicalDevice());
    initInfo.Queue = static_cast<VkQueue>(device.getGraphicsQueue());
    initInfo.DescriptorPool = static_cast<VkDescriptorPool>(imguiPool);
    initInfo.MinImageCount = swapchain.getMinImageCount();
    initInfo.ImageCount = swapchain.getMinImageCount();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, swapchain.getUIRenderPass());

    std::array<vk::CommandBuffer, 1> tempCommandBuffer;

    for (auto& commandBuffer : tempCommandBuffer)
    {
        device.beginSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_CreateFontsTexture(static_cast<VkCommandBuffer>(commandBuffer));
        device.endSingleTimeCommand(commandBuffer);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

}

void TriangleUI::draw()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    beginUIFrame();
}

void TriangleUI::renderFrame(uint32_t& imageIndex, uint32_t& currentFrame)
{   
    ImGui::Render();
    uiCommandBuffers[currentFrame].reset();
    // No need to reset command pool because the ui command buffer is using the same command pool
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    uiCommandBuffers[currentFrame].begin(beginInfo);

    std::array<vk::ClearValue, 1> clearValues;
    clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>({{0.0f, 0.0f, 0.0f, 1.0f}})));

    vk::RenderPassBeginInfo renderPassBeginInfo(
        swapchain.getUIRenderPass(), 
        swapchain.getUIFramebuffers()[imageIndex], 
        {{0, 0}, swapchain.getExtent()}, 
        clearValues);

    uiCommandBuffers[currentFrame].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), uiCommandBuffers[currentFrame]);

    uiCommandBuffers[currentFrame].endRenderPass();
    uiCommandBuffers[currentFrame].end();
}