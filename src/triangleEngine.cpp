#include "triangleEngine.hpp"
#include "triangleCamera.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "trianglePipeline.hpp"
#include "triangleSwapchain.hpp"
#include "triangleUI.hpp"
#include "triangleRenderer.hpp"
#include "vulkan/vulkan_enums.hpp"

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
    // NOTE: Cannot seperate initialization with main loop because main loop will get different memory addres than the initialization ones.
    
    triangleModel = std::make_unique<TriangleModel>(triangleDevice);
    triangleModel->createUniformBuffers(triangleSwapchain.MAX_FRAMES_IN_FLIGHT);

    triangleDescriptor = std::make_unique<TriangleDescriptor>(triangleDevice, triangleSwapchain.MAX_FRAMES_IN_FLIGHT, triangleModel->getUniformBuffers());
    
    // Adding entity
    TriangleModel::Mesh cubeMesh = TriangleModel::Mesh(cubeVertices, cubeIndices), squareMesh = TriangleModel::Mesh(squareVertices, squareIndices);
    TriangleModel::Transform cubeTransform, squareTransform;
    squareTransform.position = glm::vec3(2.f, 2.f, 2.f);

    TriangleModel::RenderModel cubeModel = TriangleModel::RenderModel(cubeMesh, cubeTransform), 
                               squareModel = TriangleModel::RenderModel(squareMesh, squareTransform);

    ecs = std::make_unique<triangle::ECS>();
    triangle::Entity cubeEntity = triangle::Entity(), squareEntity = triangle::Entity();

    ecs->addEntity(cubeEntity);
    ecs->addEntity(squareEntity);

    ecs->assignComponent<TriangleModel::RenderModel>(cubeEntity, cubeModel);
    ecs->assignComponent<TriangleModel::RenderModel>(squareEntity, squareModel);

    createPipelineLayout();
    createPipeline();

    triangleRenderer = std::make_unique<triangle::Renderer>(triangleDevice, triangleSwapchain, triangleUI);

    triangleCamera = std::make_unique<TriangleCamera>(triangleWindow.getWindow(), WIDTH, HEIGHT);
    triangleCamera->setCamera(cameraPos, glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

    initSceneSystem();

    uint32_t currentFrame = 0, imageIndex;
    vk::DeviceSize dynamicOffset = 0;

    // Main loop
    while (!triangleWindow.shouldClose())
    {
        frame = (frame + 1) % 100; 
        glfwPollEvents();

        triangleUI.draw([this]
                        { drawUI(); });

        triangleCamera->processCameraMovement();
        if (triangleUI.isCapturingMouse() == false)
        {
            triangleCamera->processCameraRotation(0.2f);
        }

        triangleRenderer->beginCommandBuffer();
        triangleRenderer->beginRenderPass();

        auto currentCommandBuffer = triangleRenderer->getCurrentCommandBuffer();

        trianglePipeline.bind(currentCommandBuffer);

        mvpSystem(triangleRenderer->getCurrentFrame());

        for (const auto &entity : ecs->getEntities())
        {
            auto component = ecs->getComponent<TriangleModel::RenderModel>(entity);
            dynamicOffset = (entity.id - 1) * triangleModel->getDynamicAlignment();

            if (component == nullptr)
                continue;
                
            currentCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, triangleDescriptor->getDescriptorSet(currentFrame), dynamicOffset);
            triangleModel->bind(currentCommandBuffer, 
                sizeof(component->mesh.vertices[0]) * component->mesh.vertices.size() * (entity.id - 1), 
                sizeof(component->mesh.indices[0]) * component->mesh.indices.size() * (entity.id - 1));

            TriangleModel::MeshPushConstant push{};
            push.offset = {0.0f + (frame * 0.005f * entity.id * entity.id), 0.0f, 0.0f + (frame * 0.005f * entity.id * entity.id)};
            push.color = {0.0f, 0.0f + (frame * 0.005f), 0.0f + (frame * 0.0025f)};

            currentCommandBuffer.pushConstants<TriangleModel::MeshPushConstant>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, push);

            currentCommandBuffer.drawIndexed(static_cast<uint32_t>(component->mesh.indices.size()), 1, 0, 0, 0);
        }
        triangleRenderer->endRenderpass();
        triangleRenderer->endCommandBuffer();

        triangleRenderer->submitBuffer();
    }

    triangleDevice.getLogicalDevice().waitIdle();
    
    cleanup();
}

void TriangleEngine::drawUI()
{
    float velocity = 0.25f;
    // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    char entityID[50];

    if (ImGui::Begin("Entity"))
    {
        for (const auto &entity : ecs->getEntities())
        {
            sprintf(entityID, "Entity %d", entity.id);
            auto component = ecs->getComponent<TriangleModel::RenderModel>(entity);
            if (component == nullptr) continue;

            // ImGui::BeginGroup();
            ImGui::BeginChild(entityID, ImVec2(0, 100.f), true);
            ImGui::Text("Entity ID: %d\nTransform component: %p", entity.id, std::addressof(component->transform.position));
            ImGui::Text("translation: (%.3f, %.3f, %.3f)", component->transform.position.x, component->transform.position.y, component->transform.position.z);
            // ImGui::SliderFloat("pos x", &component->transform.position.x, 0.f, 10.0f);
            // ImGui::SliderFloat("pos y", &component->transform.position.y, 0.f, 10.0f);
            // ImGui::SliderFloat("pos z", &component->transform.position.z, 0.f, 10.0f);
            ImGui::SliderFloat3("position", &(component->transform.position.x), 0.f, 10.f);
            ImGui::EndChild();
        }
        
        ImGui::End();
    }

    if (ImGui::Begin("Camera"))
    {
        ImGui::BeginGroup();
        auto cameraProperty = triangleCamera->getCameraProperty();
        ImGui::Text("Cam pos: (%f, %f, %f)", cameraProperty.cameraPos.x, cameraProperty.cameraPos.y, cameraProperty.cameraPos.z);
        ImGui::Text("Cam front: (%f, %f, %f)", cameraProperty.cameraFront.x, cameraProperty.cameraFront.y, cameraProperty.cameraFront.z);
        ImGui::Text("Cam up: (%f, %f, %f)", cameraProperty.cameraUp.x, cameraProperty.cameraUp.y, cameraProperty.cameraUp.z);
        ImGui::Text("Cam direction: (%f, %f, %f)", cameraProperty.direction.x, cameraProperty.direction.y, cameraProperty.direction.z);

        ImGui::EndGroup();
        ImGui::End();
    }

    if (ImGui::Begin("Device Properties"))
    {
        ImGui::Text("UBO dynamic offset: %d", triangleModel->getDynamicAlignment());
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

void TriangleEngine::cleanup()
{
    triangleDevice.getLogicalDevice().destroyPipelineLayout(pipelineLayout);
    // trianglePipeline.destroyShaderModule();
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

    trianglePipeline.createGraphicsPipeline(pipelineConfig, "shaders/vert.spv", "shaders/frag.spv");
}

void TriangleEngine::mvpSystem(uint32_t currentImage)
{
    for (auto& entity : ecs->getEntities())
    {
        auto component = ecs->getComponent<TriangleModel::RenderModel>(entity);

        if (component == nullptr) continue;

        vk::DeviceSize dynamicOffset = (entity.id - 1) * triangleModel->getDynamicAlignment();

        glm::mat4 transform = glm::translate(glm::mat4{1.0f}, component->transform.position);

        component->mesh.mvp.model = transform;
        
        cameraPos.x = sin(glfwGetTime()) * 10.0f;
        cameraPos.z = cos(glfwGetTime()) * 10.0f;

        component->mesh.mvp.view = triangleCamera->getView();

        component->mesh.mvp.proj = glm::perspective(glm::radians(45.0f), triangleSwapchain.getExtent().width / static_cast<float>(triangleSwapchain.getExtent().height), 0.1f, 20.0f);
        component->mesh.mvp.proj[1][1] *= -1;

        void* data;
        data = triangleDevice.getLogicalDevice().mapMemory(triangleModel->getUniformBufferMemory(currentImage), dynamicOffset, sizeof(component->mesh.mvp));
        memcpy(data, &component->mesh.mvp, sizeof(component->mesh.mvp));
        triangleDevice.getLogicalDevice().unmapMemory(triangleModel->getUniformBufferMemory(currentImage));
    }
}

void TriangleEngine::initSceneSystem()
{
    auto entities = ecs->getEntities();

    std::vector<std::vector<TriangleModel::Vertex>> vertexList{};
    vertexList.reserve(entities.size());

    std::vector<std::vector<Index>> indexList{};
    indexList.reserve(entities.size());

    for (const auto& entity : entities)
    {
        if (auto component = ecs->getComponent<TriangleModel::RenderModel>(entity))
        {
            vertexList.push_back(component->mesh.vertices);
            indexList.push_back(component->mesh.indices);
        }
    }
    triangleModel->allocVertexBuffer(vertexList);
    triangleModel->allocIndexBuffer(indexList);
}