#include "triangleEngine.hpp"
#include "triangleCamera.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"
#include "triangleRenderer.hpp"

#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui.h>

#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <vulkan/vulkan.hpp>

#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <chrono>

static uint32_t frame = 0;

namespace triangle
{
    
    Engine::~Engine()
    {
        triangleDevice.getLogicalDevice().destroyPipelineLayout(pipelineLayout);
    }

    void Engine::run()
    {
        // Adding entity
        Mesh cubeMesh = Mesh(cubeVertices, cubeIndices), 
                            squareMesh = Mesh(squareVertices, squareIndices);
                            // pyramidMesh = Mesh(pyramidVertices, pyramidIndices);

        Transform cubeTransform, squareTransform, pyramidTransform;
        squareTransform.position = glm::vec3(2.f, 2.f, 2.f);
        // pyramidTransform.position = glm::vec3(4.f, 4.f, 4.f);

        RenderModel cubeModel = RenderModel(cubeMesh, cubeTransform),
                                squareModel = RenderModel(squareMesh, squareTransform);
                                //    pyramidModel = RenderModel(pyramidMesh, pyramidTransform)

        triangle::Entity cubeEntity = triangle::Entity(), 
                        squareEntity = triangle::Entity();
                        //  pyramidEntity = triangle::Entity()

        ecs.addEntity(cubeEntity);
        ecs.addEntity(squareEntity);
        // ecs.addEntity(pyramidEntity);

        ecs.assignComponent<RenderModel>(cubeEntity, cubeModel);
        ecs.assignComponent<RenderModel>(squareEntity, squareModel);
        // ecs.assignComponent<TriangleModel::RenderModel>(pyramidEntity, pyramidModel);

        triangleModel = std::make_unique<Model>(triangleDevice);
        triangleModel->createUniformBuffers(triangleRenderer.getMaxFramesInFlight(), ecs.getEntitySize());

        triangleDescriptor = std::make_unique<Descriptor>(triangleDevice, triangleRenderer.getMaxFramesInFlight(), triangleModel->getUniformBuffers(), triangleRenderer.getTextureProperties());

        createPipelineLayout();
        createPipeline();


        triangleCamera = std::make_unique<TriangleCamera>(triangleWindow.getWindow(), WIDTH, HEIGHT);
        triangleCamera->setCamera(cameraPos, glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

        initSceneSystem();

        while (!triangleWindow.shouldClose())
        {
            frame = (frame + 1) % 100;
            glfwPollEvents();

            triangleRenderer.createUI([this]
                            { drawUI(); });

            triangleCamera->processCameraMovement();
            if (triangleRenderer.getUiIO().WantCaptureMouse == false)
            {
                triangleCamera->processCameraRotation(0.2f);
            }

            if (auto currentCommandBuffer = triangleRenderer.beginCommandBuffer())
            {
                triangleRenderer.beginRenderPass();

                trianglePipeline.bind(currentCommandBuffer);

                renderSystem(triangleRenderer.getCurrentFrame(), currentCommandBuffer);

                triangleRenderer.endRenderpass();
                triangleRenderer.endCommandBuffer();

                triangleRenderer.submitBuffer();
            }
        }

        triangleDevice.getLogicalDevice().waitIdle();
    }

    void Engine::drawUI()
    {
        float velocity = 0.25f;
        // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        char entityID[50];

        if (ImGui::Begin("Entity"))
        {
            for (const auto &entity : ecs.getEntities())
            {
                sprintf(entityID, "Entity %d", entity.id);
                auto component = ecs.getComponent<RenderModel>(entity);
                if (component == nullptr) continue;

                ImGui::BeginChild(entityID, ImVec2(0, 100.f), true);
                ImGui::Text("Entity ID: %d\nTransform component: %p", entity.id, std::addressof(component->transform.position));
                ImGui::Text("translation: (%.3f, %.3f, %.3f)", component->transform.position.x, component->transform.position.y, component->transform.position.z);

                ImGui::SliderFloat3("position", &(component->transform.position.x), 0.f, 10.f);
                ImGui::EndChild();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Camera"))
        {
            ImGui::BeginGroup();
            auto cameraProperty = triangleCamera->getCameraProperty();
            ImGui::Text("Cam pos: (%f, %f, %f)", cameraProperty.cameraPos.x, cameraProperty.cameraPos.y, cameraProperty.cameraPos.z);
            ImGui::Text("Cam front: (%f, %f, %f)", cameraProperty.cameraFront.x, cameraProperty.cameraFront.y, cameraProperty.cameraFront.z);
            ImGui::Text("Cam up: (%f, %f, %f)", cameraProperty.cameraUp.x, cameraProperty.cameraUp.y, cameraProperty.cameraUp.z);
            ImGui::Text("Cam direction: (%f, %f, %f)", cameraProperty.direction.x, cameraProperty.direction.y, cameraProperty.direction.z);

            ImGui::EndGroup();
        }
        ImGui::End();

        if (ImGui::Begin("Device Properties"))
        {
            ImGui::Text("UBO dynamic offset: %d", triangleModel->getDynamicAlignment());
        }
        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void Engine::createPipelineLayout()
    {
        vk::PushConstantRange pushConstant(
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(MeshPushConstant)
        );

        std::array<vk::DescriptorSetLayout, 1> descriptorSetLayout = { triangleDescriptor->getDescriptorSetLayout() };

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            vk::PipelineLayoutCreateFlags(),
            descriptorSetLayout,
            pushConstant
        );

        pipelineLayout = triangleDevice.getLogicalDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    }

    void Engine::createPipeline()
    {
        vk::Viewport viewport(0.0f, 0.0f, 0.f, 0.f , 0.0f, 1.0f);
        vk::Rect2D scissor({0, 0}, {0, 0});

        vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | 
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

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        Pipeline::PipelineConfig pipelineConfig{
            Vertex::getBindingDesciptions(),
            Vertex::getAttributeDescriptions(),
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
            vk::PipelineDynamicStateCreateInfo(
                vk::PipelineDynamicStateCreateFlags(),
                dynamicStates
            ),
            pipelineLayout,
            triangleRenderer.getMainRenderPass()
        };

        trianglePipeline.createGraphicsPipeline(pipelineConfig, "../shaders/vert.spv", "../shaders/frag.spv");
    }

    void Engine::renderSystem(uint32_t currentImage, vk::CommandBuffer& currentCommandBuffer)
    {
        vk::DeviceSize dynamicOffset = 0;
        vk::DeviceSize vertexOffset = 0, indexOffset = 0;
        for (const auto &entity : ecs.getEntities())
        {
            if (auto component = ecs.getComponent<RenderModel>(entity))
            {
                dynamicOffset = (entity.id - 1) * triangleModel->getDynamicAlignment();
                glm::mat4 transform = glm::translate(glm::mat4{1.0f}, component->transform.position);

                cameraPos.x = sin(glfwGetTime()) * 10.0f;
                cameraPos.z = cos(glfwGetTime()) * 10.0f;

                component->mesh.mvp.model = transform;
                component->mesh.mvp.view = triangleCamera->getView();
                component->mesh.mvp.proj = glm::perspective(glm::radians(45.0f), triangleRenderer.getAspectRatio(), 0.1f, 20.0f);

                component->mesh.mvp.proj[1][1] *= -1;

                void *data;
                data = triangleDevice.getLogicalDevice().mapMemory(triangleModel->getUniformBufferMemory(currentImage), dynamicOffset, sizeof(component->mesh.mvp));
                memcpy(data, &component->mesh.mvp, sizeof(component->mesh.mvp));
                triangleDevice.getLogicalDevice().unmapMemory(triangleModel->getUniformBufferMemory(currentImage));

                currentCommandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, triangleDescriptor->getDescriptorSet(currentFrame), dynamicOffset);

                triangleModel->bind(currentCommandBuffer,
                                    sizeof(component->mesh.vertices[0]) * component->mesh.vertices.size() * (entity.id - 1),
                                    sizeof(component->mesh.indices[0]) * component->mesh.indices.size() * (entity.id - 1));

                MeshPushConstant push{};
                // push.offset = {0.0f + (frame * 0.005f * entity.id * entity.id), 0.0f, 0.0f + (frame * 0.005f * entity.id * entity.id)};
                // push.color = {0.0f, 0.0f + (frame * 0.005f), 0.0f + (frame * 0.0025f)};
                push.offset = {0.f, 0.f, 0.f};
                push.color = {0.f, 0.f, 0.f};
                
                currentCommandBuffer.pushConstants<MeshPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, push);
                currentCommandBuffer.drawIndexed(static_cast<uint32_t>(component->mesh.indices.size()), 1, 0, 0, 0);
            }
        }
    }

    void Engine::initSceneSystem()
    {
        auto entities = ecs.getEntities();

        std::vector<std::vector<Vertex>> vertexList{};
        vertexList.reserve(entities.size());

        std::vector<std::vector<Index>> indexList{};
        indexList.reserve(entities.size());

        for (const auto& entity : entities)
        {
            if (auto component = ecs.getComponent<RenderModel>(entity))
            {
                vertexList.push_back(component->mesh.vertices);
                indexList.push_back(component->mesh.indices);
            }
        }
        triangleModel->allocVertexBuffer(vertexList);
        triangleModel->allocIndexBuffer(indexList);
    }
}