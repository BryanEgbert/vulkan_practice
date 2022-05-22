#include "triangleUI.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
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
    createUIDescriptorPool();
}

TriangleUI::~TriangleUI()
{
    device.getLogicalDevice().destroyDescriptorPool(imguiPool);
}

void TriangleUI::initUI()
{
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

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
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, swapchain.getMainRenderPass());

    device.beginSingleTimeCommands(uiCommandBuffer);
    ImGui_ImplVulkan_CreateFontsTexture(uiCommandBuffer);
    device.endSingleTimeCommand(uiCommandBuffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}