#include "triangleEngine.hpp"
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui.h>
#include "triangleCamera.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"
#include "triangleUI.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <chrono>

static uint32_t frame = 0;

TriangleEngine::TriangleEngine()
{
    
    triangleCamera = std::make_unique<TriangleCamera>(triangleWindow.getWindow(), WIDTH, HEIGHT);
    triangleModel = std::make_unique<TriangleModel>(triangleDevice, cubeVertices, cubeIndices);
    triangleModel->createUniformBuffers(triangleSwapchain.MAX_FRAMES_IN_FLIGHT);

    triangleDescriptor = std::make_unique<TriangleDescriptor>(triangleDevice, triangleSwapchain.MAX_FRAMES_IN_FLIGHT, triangleModel->getUniformBuffers());   

    createPipelineLayout();
    createPipeline();

    createCommandBuffer();
}

TriangleEngine::~TriangleEngine()
{
    std::cout << "Destroying pipeline layout\n"; 
    triangleDevice.getLogicalDevice().destroyPipelineLayout(pipelineLayout);
}

void TriangleEngine::drawUI()
{
    float velocity = 0.25f;
    // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    if (ImGui::Begin("Test"))
    {
        ImGui::BeginGroup();
        ImGui::SliderFloat("pos x", &BoxPosition.x, 0.1f, 10.0f);
        ImGui::SliderFloat("pos y", &BoxPosition.y, 0.1f, 10.0f);
        ImGui::SliderFloat("pos z", &BoxPosition.z, 0.1f, 10.0f);
        ImGui::EndGroup();

        ImGui::BeginGroup();
        auto cameraProperty = triangleCamera->getCameraProperty();
        ImGui::Text("Cam pos: (%f, %f, %f)", cameraProperty.cameraPos.x, cameraProperty.cameraPos.y, cameraProperty.cameraPos.z);
        ImGui::Text("Cam front: (%f, %f, %f)", cameraProperty.cameraFront.x, cameraProperty.cameraFront.y, cameraProperty.cameraFront.z);
        ImGui::Text("Cam up: (%f, %f, %f)", cameraProperty.cameraUp.x, cameraProperty.cameraUp.y, cameraProperty.cameraUp.z);
        ImGui::Text("Cam direction: (%f, %f, %f)", cameraProperty.direction.x, cameraProperty.direction.y, cameraProperty.direction.z);

        ImGui::EndGroup();

        ImGui::End();
    }

    if (ImGui::Begin("Scene"))
    {
        // ImGui::Text("Camera");

        // ImGui::Image(ImTextureID user_texture_id, const ImVec2 &size)
        ImGui::End();
    }

    ImGui::ShowDemoWindow();
}

void TriangleEngine::run()
{
    uint32_t currentFrame = 0, imageIndex;
    triangleCamera->setCamera(cameraPos, glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

    while(!triangleWindow.shouldClose())
    {
        triangleSwapchain.acquireNextImage(&imageIndex, currentFrame);

        glfwPollEvents();
        
        triangleUI.draw([this]{ drawUI(); });

        triangleCamera->processCameraMovement();
        if (triangleUI.isCapturingMouse() == false) 
        {
            triangleCamera->processCameraRotation(0.2f);
        }

        triangleUI.renderFrame(imageIndex, currentFrame);
        drawFrames(imageIndex, currentFrame);

        currentFrame = (currentFrame + 1) % triangleSwapchain.MAX_FRAMES_IN_FLIGHT;
    }

    triangleDevice.getLogicalDevice().waitIdle();
}

void TriangleEngine::createPipelineLayout()
{
    vk::PushConstantRange pushConstant(
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(TriangleModel::MeshPushConstant)
    );

    std::array<vk::DescriptorSetLayout, 1> descriptorSetLayout = { triangleDescriptor->getDescriptorSetLayout() };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(),
        descriptorSetLayout,
        pushConstant
    );

    pipelineLayout = triangleDevice.getLogicalDevice().createPipelineLayout(pipelineLayoutCreateInfo);
}

void TriangleEngine::createPipeline()
{
    vk::Viewport viewport(0.0f, 0.0f, (float)triangleSwapchain.getExtent().width, (float)triangleSwapchain.getExtent().height, 0.0f, 1.0f);
    vk::Rect2D scissor({0, 0}, triangleSwapchain.getExtent());

    vk::ColorComponentFlags colorComponentFlags( vk::ColorComponentFlagBits::eR | 
                                            vk::ColorComponentFlagBits::eG | 
                                            vk::ColorComponentFlagBits::eB | 
                                            vk::ColorComponentFlagBits::eA);
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
        false, 
        vk::BlendFactor::eOne, 
        vk::BlendFactor::eZero, 
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, 
        colorComponentFlags);

    TrianglePipeline::PipelineConfig pipelineConfig{
        TriangleModel::Vertex::getBindingDesciptions(),
        TriangleModel::Vertex::getAttributeDescriptions(),
        vk::PipelineInputAssemblyStateCreateInfo(
            vk::PipelineInputAssemblyStateCreateFlags(), 
            vk::PrimitiveTopology::eTriangleList, false
        ),
        vk::PipelineViewportStateCreateInfo(
            vk::PipelineViewportStateCreateFlags(), viewport, scissor
        ),
        vk::PipelineRasterizationStateCreateInfo(
            vk::PipelineRasterizationStateCreateFlags(), 
            false, 
            false, 
            vk::PolygonMode::eFill, 
            vk::CullModeFlagBits::eNone, 
            vk::FrontFace::eCounterClockwise, 
            false, 
            0.0f, 
            0.0f, 
            0.0f, 
            1.0f
        ),
        vk::PipelineMultisampleStateCreateInfo(
            vk::PipelineMultisampleStateCreateFlags(),
            vk::SampleCountFlagBits::e1, 
            false, 
            1.0f,
            nullptr,
            false,
            false
        ),
        vk::PipelineDepthStencilStateCreateInfo(
            vk::PipelineDepthStencilStateCreateFlags(),
            true,
            true, 
            vk::CompareOp::eLess, 
            false, 
            false
        ),
        vk::PipelineColorBlendStateCreateInfo(
            vk::PipelineColorBlendStateCreateFlags(),
            false,
            vk::LogicOp::eCopy,
            colorBlendAttachmentState,
            {{1.0f, 1.0f, 1.0f, 1.0f}}
        ),
        std::nullopt,
        pipelineLayout,
        triangleSwapchain.getMainRenderPass()
    };

    trianglePipeline = std::make_unique<TrianglePipeline>(triangleDevice, pipelineConfig, "shaders/vert.spv", "shaders/frag.spv");
}

void TriangleEngine::createCommandBuffer()
{
    commandBuffers.resize(triangleSwapchain.MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo cmdBufferAllocateInfo(
        triangleDevice.getCommandPool(),
        vk::CommandBufferLevel::ePrimary,
        commandBuffers.size()
    );

    commandBuffers = triangleDevice.getLogicalDevice().allocateCommandBuffers(cmdBufferAllocateInfo);
}

void TriangleEngine::updateUniformBuffer(uint32_t currentImage)
{
    glm::mat4 transform = glm::translate(glm::mat4{1.0f}, {BoxPosition.x, BoxPosition.y, BoxPosition.z});
    // transform = transform * glm::eulerAngleXYZ(0.5f + time, 0.5f + time, 0.5f + time);

    // ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
    ubo.model = transform;
    
    cameraPos.x = sin(glfwGetTime()) * 10.0f;
    cameraPos.z = cos(glfwGetTime()) * 10.0f;
    ubo.view = triangleCamera->getView();
    // ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.proj = glm::perspective(glm::radians(45.0f), triangleSwapchain.getExtent().width / (float) triangleSwapchain.getExtent().height, 0.1f, 20.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    data = triangleDevice.getLogicalDevice().mapMemory(triangleModel->getUniformBufferMemory(currentImage), 0, sizeof(ubo));
        memcpy(data, &ubo, sizeof(ubo));
    triangleDevice.getLogicalDevice().unmapMemory(triangleModel->getUniformBufferMemory(currentImage));
}

void TriangleEngine::recordCommandBuffer(vk::CommandBuffer& cmdBuffer, uint32_t imageIndex, uint32_t& currentFrame)
{
    frame = (frame + 1) % 100;

    vk::CommandBufferBeginInfo cmdBufferBeginInfo(vk::CommandBufferUsageFlags(), nullptr);
    cmdBuffer.begin(cmdBufferBeginInfo);

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>({{0.0f, 0.0f, 0.0f, 1.0f}})));
    clearValues[1].setDepthStencil({1.0f, 0});

    vk::RenderPassBeginInfo renderPassBeginInfo(
        triangleSwapchain.getMainRenderPass(),
        triangleSwapchain.getMainFramebuffers()[imageIndex],
        {{0, 0}, triangleSwapchain.getExtent()},
        clearValues);

    cmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    trianglePipeline->bind(cmdBuffer);

    triangleModel->bind(cmdBuffer);

    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, triangleDescriptor->getDescriptorSet(currentFrame), nullptr);

    TriangleModel::MeshPushConstant push{};
    push.offset = {0.0f, 0.0f, 0.0f};
    push.color = {0.0f, 0.0f + (frame * 0.005f), 0.0f + (frame * 0.0025f)};

    cmdBuffer.pushConstants<TriangleModel::MeshPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, push);        

    cmdBuffer.drawIndexed(static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

    cmdBuffer.endRenderPass();

    cmdBuffer.end();
}

void TriangleEngine::drawFrames(uint32_t& imageIndex, uint32_t& currentFrame)
{
    commandBuffers[currentFrame].reset();
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex, currentFrame);

    std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    std::array<vk::Semaphore, 1> waitSemaphore = {triangleSwapchain.getPresentSemaphore(currentFrame)};
    std::array<vk::Semaphore, 1> signalSemaphore = {triangleSwapchain.getRenderSemaphore(currentFrame)};

    std::array<vk::CommandBuffer, 2> currentCommandBuffer = {commandBuffers[currentFrame], triangleUI.getCommandBuffers(currentFrame)};

    updateUniformBuffer(currentFrame);

    vk::SubmitInfo submitInfo(waitSemaphore, waitStages, currentCommandBuffer, signalSemaphore);
    std::vector<vk::SubmitInfo> submitInfos = {submitInfo};

    triangleDevice.getGraphicsQueue().submit(submitInfos, triangleSwapchain.getInFlightFences(currentFrame));

    std::array<vk::SwapchainKHR, 1> swapchains = {triangleSwapchain.getSwapchain()};

    vk::PresentInfoKHR presentInfo(signalSemaphore, swapchains, imageIndex);

    if (triangleDevice.getPresentQueue().presentKHR(presentInfo) != vk::Result::eSuccess)
        throw std::runtime_error("Something's wrong when presenting");
}