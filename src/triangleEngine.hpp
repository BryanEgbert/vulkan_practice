#pragma once

#include "triangleCamera.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "trianglePipeline.hpp"
#include "triangleRenderer.hpp"
#include "triangleSwapchain.hpp"
#include "triangleUI.hpp"
#include "triangleWindow.hpp"
#include "triangleDescriptor.hpp"

#include <memory>
#include <vector>


class TriangleEngine
{
public:    
    static constexpr int WIDTH = 1920;
    static constexpr int HEIGHT = 1200;

    TriangleEngine();
    ~TriangleEngine();

    void run();
private:
    TriangleModel::MVP ubo{};

    struct BoxPosition {
        float x = 0.f, y = 0.f, z = 0.f;
    } BoxPosition;

    std::vector<TriangleModel::Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}}
    };

    std::vector<TriangleModel::Vertex> cubeVertices = {
        // Front
        {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.4f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.3f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.2f, 0.2f, 0.0f}},
        // Base
        {{-0.5f, -0.5f, -0.5f}, {0.6f, 0.2f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        {{0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        {{-0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        // left
        {{-0.5f, -0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        {{-0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        {{-0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        {{-0.5f, -0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        // right
        {{0.5f, -0.5f, -0.5f}, {0.5f, 0.2f, 0.5f}},
        {{0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        {{0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        {{0.5f, -0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        // top
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.2f, 0.5f}},
        {{0.5f, -0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        {{0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        {{-0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        // back
        {{-0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}},
        {{0.5f, 0.5f, -0.5f}, {0.9f, 0.2f, 0.5f}},
        {{0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
        {{-0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}},
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    std::vector<uint16_t> cubeIndices = {
        0, 1, 2, 2, 3, 0, 
        4, 5, 6, 6, 7, 4, 
        8, 9, 10, 10, 11, 8, 
        12, 13, 14, 14, 15, 12, 
        16, 17, 18, 18, 19, 16, 
        20, 21, 22, 22, 23, 20
    };

    TriangleWindow triangleWindow{WIDTH, HEIGHT, "Vulkan"};
    TriangleDevice triangleDevice{"vulkan basic", triangleWindow};
    TriangleSwapchain triangleSwapchain{triangleDevice};
    TriangleUI triangleUI{triangleDevice, triangleWindow, triangleSwapchain};

    vk::PipelineLayout pipelineLayout;
    std::unique_ptr<TrianglePipeline> trianglePipeline;
    std::unique_ptr<TriangleModel> triangleModel;
    std::unique_ptr<TriangleDescriptor> triangleDescriptor;
    std::unique_ptr<TriangleCamera> triangleCamera;
    std::vector<vk::CommandBuffer> commandBuffers;

    glm::vec3 cameraPos = glm::vec3(0.f, 0.f, 2.f);

    // void newFrame();
    void drawUI();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffer();
    void updateUniformBuffer(uint32_t currentImage);
    void recordCommandBuffer(vk::CommandBuffer& cmdBuffer, uint32_t imageIndex, uint32_t& currentFrame);
    void drawFrames(uint32_t& imageIndex, uint32_t& currentFrame);
};