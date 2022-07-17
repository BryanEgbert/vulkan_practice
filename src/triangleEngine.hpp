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
#include "triangleECS.hpp"

#include <memory>
#include <vector>

using Index = uint32_t;

class TriangleEngine
{
public:    
    static constexpr int WIDTH = 1920;
    static constexpr int HEIGHT = 1280;

    ~TriangleEngine();

    void run();

private:
    std::vector<TriangleModel::Vertex> squareVertices = {
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

    std::vector<Index> squareIndices = {
        0, 1, 2, 2, 3, 0
    };

    std::vector<Index> cubeIndices = {
        0, 1, 2, 2, 3, 0, 
        4, 5, 6, 6, 7, 4, 
        8, 9, 10, 10, 11, 8, 
        12, 13, 14, 14, 15, 12, 
        16, 17, 18, 18, 19, 16, 
        20, 21, 22, 22, 23, 20
    };

    uint32_t currentFrame = 0, imageIndex;
    vk::DeviceSize dynamicOffset = 0;
    glm::vec3 cameraPos = glm::vec3(0.f, 0.f, 2.f);

    TriangleWindow triangleWindow{WIDTH, HEIGHT, "Vulkan"};
    TriangleDevice triangleDevice{"vulkan basic", triangleWindow};
    TriangleSwapchain triangleSwapchain{triangleDevice};
    TriangleUI triangleUI{triangleDevice, triangleWindow, triangleSwapchain};
    TrianglePipeline trianglePipeline{triangleDevice};
    triangle::ECS ecs;

    vk::PipelineLayout pipelineLayout;
    
    std::unique_ptr<triangle::Renderer> triangleRenderer;
    std::unique_ptr<TriangleModel> triangleModel;
    std::unique_ptr<TriangleDescriptor> triangleDescriptor;
    std::unique_ptr<TriangleCamera> triangleCamera;

    void createPipelineLayout();
    void createPipeline();

    void initSceneSystem();
    void mvpSystem(uint32_t currentImage);
    void renderSystem(uint32_t currentImage, vk::CommandBuffer &currentCommandBuffer);

    void drawUI();
};