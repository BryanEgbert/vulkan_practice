#include "triangleEngine.hpp"
#include "triangleCamera.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"

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
    
    // triangleModel = std::make_unique<TriangleModel>(triangleDevice, vertices, indices);
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

void TriangleEngine::run()
{
    uint32_t currentFrame = 0;
    triangleCamera->setCamera(glm::vec3(0.f, 0.f, 2.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
    while(!triangleWindow.shouldClose())
    {
        glfwPollEvents();
        
        triangleCamera->processCameraMovement();
        triangleCamera->processCameraRotation(0.2f);

        drawFrames(currentFrame);
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
        triangleSwapchain.getRenderPass()
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
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    TriangleModel::UniformBufferObject ubo{};
    glm::mat4 transform = glm::translate(glm::mat4{1.0f}, {0.0f, 0.0f, 0.0f});
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
        triangleSwapchain.getRenderPass(),
        triangleSwapchain.getFramebuffers()[imageIndex],
        {{0, 0}, triangleSwapchain.getExtent()},
        clearValues);

    cmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    trianglePipeline->bind(cmdBuffer);

    triangleModel->bind(cmdBuffer);

    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, triangleDescriptor->getDescriptorSet(currentFrame), nullptr);

    TriangleModel::MeshPushConstant push{};
    push.offset = {0.0f, 0.0f, 0.0f};
    push.color = {0.0f, 0.0f + (frame * 0.005f), 0.0f + (frame * 0.0025f)};
    // push.offset = {0.0f + (frame * 0.01f), 0.0f + (frame * 0.01f), 0.0f + (frame * 0.01f)};
    // push.color = {0.0f, 0.0f, 0.0f};

    cmdBuffer.pushConstants<TriangleModel::MeshPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, push);        

    cmdBuffer.drawIndexed(static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

    cmdBuffer.endRenderPass();

    cmdBuffer.end();
}

void TriangleEngine::drawFrames(uint32_t& currentFrame)
{
    uint32_t imageIndex;
    triangleSwapchain.acquireNextImage(&imageIndex, currentFrame);

    commandBuffers[currentFrame].reset();
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex, currentFrame);

    std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    std::array<vk::Semaphore, 1> waitSemaphore = {triangleSwapchain.getPresentSemaphore(currentFrame)};
    std::array<vk::Semaphore, 1> signalSemaphore = {triangleSwapchain.getRenderSemaphore(currentFrame)};

    std::array<vk::CommandBuffer, 1> currentCommandBuffer = {commandBuffers[currentFrame]};

    updateUniformBuffer(currentFrame);

    vk::SubmitInfo submitInfo(waitSemaphore, waitStages, currentCommandBuffer, signalSemaphore);
    std::vector<vk::SubmitInfo> submitInfos = {submitInfo};

    triangleDevice.getGraphicsQueue().submit(submitInfos, triangleSwapchain.getInFlightFences(currentFrame));

    std::array<vk::SwapchainKHR, 1> swapchains = {triangleSwapchain.getSwapchain()};

    vk::PresentInfoKHR presentInfo(signalSemaphore, swapchains, imageIndex);

    if (triangleDevice.getPresentQueue().presentKHR(presentInfo) != vk::Result::eSuccess)
        throw std::runtime_error("Something's wrong when presenting");

    currentFrame = (currentFrame + 1) % triangleSwapchain.MAX_FRAMES_IN_FLIGHT;
}