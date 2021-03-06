#pragma once

#include "triangleCamera.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "trianglePipeline.hpp"
#include "triangleRenderer.hpp"
#include "triangleSwapchain.hpp"
#include "triangleWindow.hpp"
#include "triangleDescriptor.hpp"
#include "triangleTypes.hpp"
#include "triangleECS.hpp"

#include <memory>
#include <vector>

using Index = uint32_t;

namespace triangle
{
    class Engine
    {
    public:    
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;

        ~Engine();

        void run();

    private:
        std::vector<Vertex> squareVertices = {
            // position         ,   color            , uv
            {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}, {1.f, 0.f}},
            {{0.5f, -0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.f, 0.f}},
            {{0.5f, 0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.f, 1.f}},
            {{-0.5f, 0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}, {1.f, 1.f}}
        };

        std::vector<Vertex> cubeVertices = {
            // Front
            {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.0f, 0.0f}, {1.f, 0.f}},
            {{0.5f, -0.5f, -0.5f}, {0.4f, 0.0f, 0.0f}, {0.f, 0.f}},
            {{0.5f, -0.5f, 0.5f}, {0.3f, 0.0f, 0.0f}, {0.f, 1.f}},
            {{-0.5f, -0.5f, 0.5f}, {0.2f, 0.2f, 0.0f}, {1.f, 1.f}},
            // Base
            {{-0.5f, -0.5f, -0.5f}, {0.6f, 0.2f, 0.0f}, {1.f, 0.f}},
            {{0.5f, -0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 0.f}},
            {{0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 1.f}},
            {{-0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 1.f}},
            // left
            {{-0.5f, -0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 0.f}},
            {{-0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 0.f}},
            {{-0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 1.f}},
            {{-0.5f, -0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 1.f}},
            // right
            {{0.5f, -0.5f, -0.5f}, {0.5f, 0.2f, 0.5f}, {1.f, 0.f}},
            {{0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 0.f}},
            {{0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 1.f}},
            {{0.5f, -0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 1.f}},
            // top
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.2f, 0.5f}, {1.f, 0.f}},
            {{0.5f, -0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 0.f}},
            {{0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 1.f}},
            {{-0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 1.f}},
            // back
            {{-0.5f, 0.5f, -0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 0.f}},
            {{0.5f, 0.5f, -0.5f}, {0.9f, 0.2f, 0.5f}, {0.f, 0.f}},
            {{0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {0.f, 1.f}},
            {{-0.5f, 0.5f, 0.5f}, {0.6f, 0.2f, 0.5f}, {1.f, 1.f}},
        };

        // std::vector<Vertex> pyramidVertices = {
        //     {{-0.5f, -0.5f, 0.5f}, {0.5f, 0.f, 0.f}},
        //     {{0.5f, -0.5f, 0.5f}, {0.5f, 0.f, 0.f}},
        //     {{0.f, 0.5f, 0.f}, {0.5f, 0.f, 0.f}},

        //     {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.f, 0.f}},

        //     {{0.5f, -0.5f, -0.5f}, {0.5f, 0.f, 0.f}},

        //     {{0.5f, -0.5f, -0.5f}, {0.5f, 0.f, 0.f}},
        //     {{0.5f, -0.5f, 0.5f}, {0.5f, 0.f, 0.f}},
        //     {{0.f, 0.f, 0.f}, {0.5f, 0.f, 0.f}}
        // };

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

        // std::vector<Index> pyramidIndices = {
        //     0, 1, 2,
        //     0, 3, 2,
        //     3, 4, 2, 
        //     4, 1, 2,
        //     0, 1, 3, 4
        // };

        uint32_t currentFrame = 0, imageIndex;
        vk::DeviceSize dynamicOffset = 0;
        glm::vec3 cameraPos = glm::vec3(0.f, 0.f, 2.f);

        Window triangleWindow{WIDTH, HEIGHT, "Vulkan"};
        Device triangleDevice{"vulkan basic", triangleWindow};
        Pipeline trianglePipeline{triangleDevice};
        Renderer triangleRenderer{triangleDevice, triangleWindow};
        ECS ecs;

        // vk::PipelineLayout pipelineLayout;

        std::unique_ptr<Model> triangleModel;
        std::unique_ptr<Descriptor> triangleDescriptor;
        std::unique_ptr<TriangleCamera> triangleCamera;

        std::vector<vk::PipelineLayout> layouts;
        std::vector<vk::Pipeline> pipelines;

        vk::PipelineLayout createPipelineLayout();
        void createPipeline(Pipeline::PipelineConfig& pipelineConfig);

        void initSceneSystem();
        void mvpSystem(uint32_t currentImage);
        void renderSystem(uint32_t currentImage, vk::CommandBuffer &currentCommandBuffer);
        void initEntities();

        void drawUI();
    };
}