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
static bool enableWireframe = false;

namespace triangle
{
    
    Engine::~Engine()
    {
        for (auto& pipelineLayout : layouts)
            triangleDevice.getLogicalDevice().destroyPipelineLayout(pipelineLayout);

        for (auto& pipeline : pipelines)
            triangleDevice.getLogicalDevice().destroyPipeline(pipeline);
    }

    void Engine::run()
    {
        triangleModel = std::make_unique<Model>(triangleDevice);
        triangleModel->createUniformBuffers(triangleRenderer.getMaxFramesInFlight(), 2);

        triangleDescriptor = std::make_unique<Descriptor>(triangleDevice, triangleRenderer.getMaxFramesInFlight(), triangleModel->getUniformBuffers(), triangleRenderer.get2DTextureProperties());
        triangleDescriptor->createDescriptorSets(triangleModel->getUniformBuffers(), triangleRenderer.get2DTextureProperties());
        triangleDescriptor->createCubemapDescriptorSets(triangleModel->getCubemapUniformBuffers(), triangleRenderer.getCubemapTextureProperties());

        Mesh cubeMesh = Mesh(cubeVertices, cubeIndices),
             squareMesh = Mesh(squareVertices, squareIndices),
             skyboxMesh = Mesh(skyboxVertices, skyboxIndices);


        Transform cubeTransform, squareTransform, skyboxTransform;
        squareTransform.position = glm::vec3(2.f, 2.f, 2.f);

        vk::PipelineLayout defaultPipelineLayout = createPipelineLayout();
        layouts.push_back(defaultPipelineLayout);

        vk::Pipeline defaultPipeline = trianglePipeline.createDefaultGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(defaultPipeline);

        vk::Pipeline texturedPipeline = trianglePipeline.createTextureGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(texturedPipeline);

        vk::Pipeline cubemapPipeline = trianglePipeline.createCubemapGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(cubemapPipeline);

        vk::Pipeline wireframeDefaultPipeline = trianglePipeline.createWireframeDefaultGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(wireframeDefaultPipeline);

        vk::Pipeline wireframeTexturedPipeline = trianglePipeline.createWireframeTexturedGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(wireframeTexturedPipeline);

        Material    defaultMaterial{.pipelineLayout = defaultPipelineLayout, .solidPipeline = defaultPipeline, .wireframePipeline = wireframeDefaultPipeline},
                    textureMaterial{.pipelineLayout = defaultPipelineLayout, .solidPipeline = texturedPipeline, .wireframePipeline = wireframeTexturedPipeline},
                    cubemapMaterial{.pipelineLayout = defaultPipelineLayout, .solidPipeline = cubemapPipeline};

        Transform cubemapTransform = Transform();

        skyboxObject.mesh = &skyboxMesh;
        skyboxObject.material = &cubemapMaterial;
        skyboxObject.transform = &cubemapTransform;

        RenderModel cubeModel = RenderModel(cubeMesh, cubeTransform, defaultMaterial),
                    squareModel = RenderModel(squareMesh, squareTransform, textureMaterial);

        static RenderModel skyboxObject = RenderModel(skyboxMesh, skyboxTransform, cubemapMaterial);

        triangle::Entity cubeEntity = triangle::Entity(),
                         squareEntity = triangle::Entity();

        ecs.addEntity(cubeEntity);
        ecs.addEntity(squareEntity);

        ecs.assignComponent<RenderModel>(cubeEntity, cubeModel);
        ecs.assignComponent<RenderModel>(squareEntity, squareModel);

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

                renderSystem(triangleRenderer.getCurrentFrame(), currentCommandBuffer);

                triangleRenderer.endRenderpass();
                triangleRenderer.endCommandBuffer();

                triangleRenderer.submitBuffer();
            }
        }

        triangleDevice.getLogicalDevice().waitIdle();
    }

    void Engine::initEntities()
    {
        Mesh cubeMesh = Mesh(cubeVertices, cubeIndices),
             squareMesh = Mesh(squareVertices, squareIndices);
        // pyramidMesh = Mesh(pyramidVertices, pyramidIndices);

        Transform cubeTransform, squareTransform, pyramidTransform;
        squareTransform.position = glm::vec3(2.f, 2.f, 2.f);
        // pyramidTransform.position = glm::vec3(4.f, 4.f, 4.f);

        vk::PipelineLayout defaultPipelineLayout = createPipelineLayout();
        layouts.push_back(defaultPipelineLayout);

        vk::Pipeline defaultPipeline = trianglePipeline.createDefaultGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(defaultPipeline);

        vk::Pipeline texturedPipeline = trianglePipeline.createTextureGraphicsPipeline(defaultPipelineLayout, triangleRenderer.getMainRenderPass());
        pipelines.push_back(texturedPipeline);

        Material defaultMaterial{.pipelineLayout = defaultPipelineLayout, .solidPipeline = defaultPipeline},
            textureMaterial{.pipelineLayout = defaultPipelineLayout, .solidPipeline = texturedPipeline};

        RenderModel cubeModel = RenderModel(cubeMesh, cubeTransform, defaultMaterial),
                    squareModel = RenderModel(squareMesh, squareTransform, textureMaterial);
        //    pyramidModel = RenderModel(pyramidMesh, pyramidTransform)

        triangle::Entity cubeEntity = triangle::Entity(),
                         squareEntity = triangle::Entity();
        //  pyramidEntity = triangle::Entity()

        ecs.addEntity(cubeEntity);
        ecs.addEntity(squareEntity);
        // ecs.addEntity(pyramidEntity);

        ecs.assignComponent<RenderModel>(cubeEntity, cubeModel);
        ecs.assignComponent<RenderModel>(squareEntity, squareModel);
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
            ImGui::Checkbox("Wireframe", &enableWireframe);
        }
        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    vk::PipelineLayout Engine::createPipelineLayout()
    {
        std::array<vk::DescriptorSetLayout, 1> descriptorSetLayout = { triangleDescriptor->getDescriptorSetLayout() };

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            vk::PipelineLayoutCreateFlags(),
            descriptorSetLayout,
            nullptr
        );

        return triangleDevice.getLogicalDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    }

    void Engine::createPipeline(Pipeline::PipelineConfig &pipelineConfig)
    {
        trianglePipeline.createGraphicsPipeline(pipelineConfig, "../shaders/vert.spv", "../shaders/frag.spv");
    }

    void Engine::renderSystem(uint32_t currentImage, vk::CommandBuffer& currentCommandBuffer)
    {
        vk::DeviceSize dynamicOffset = 0;
        vk::DeviceSize vertexOffset = 0, indexOffset = 0;

        static vk::Pipeline* lastPipeline = nullptr;

        for (const auto &entity : ecs.getEntities())
        {
            if (auto renderComponent = ecs.getComponent<RenderModel>(entity))
            {
                // update uniforms
                glm::mat4 transform = glm::translate(glm::mat4{1.0f}, renderComponent->transform.position);

                cameraPos.x = sin(glfwGetTime()) * 10.0f;
                cameraPos.z = cos(glfwGetTime()) * 10.0f;

                renderComponent->mesh.mvp.model = transform;
                renderComponent->mesh.mvp.view = triangleCamera->getView();
                renderComponent->mesh.mvp.proj = glm::perspective(glm::radians(45.0f), triangleRenderer.getAspectRatio(), 0.1f, 100.0f);

                renderComponent->mesh.mvp.proj[1][1] *= -1;

                void *data;
                data = triangleDevice.getLogicalDevice().mapMemory(triangleModel->getUniformBufferMemory(currentImage), dynamicOffset, sizeof(renderComponent->mesh.mvp));
                memcpy(data, &renderComponent->mesh.mvp, sizeof(renderComponent->mesh.mvp));
                triangleDevice.getLogicalDevice().unmapMemory(triangleModel->getUniformBufferMemory(currentImage));

                // skyboxObject.mesh->mvp.view = glm::mat4(glm::mat3(triangleCamera->getView()));
                skyboxObject.mesh->mvp.view = glm::mat4(glm::mat3(triangleCamera->getView()));
                skyboxObject.mesh->mvp.proj = glm::perspective(glm::radians(45.0f), triangleRenderer.getAspectRatio(), 0.1f, 100.0f);

                data = triangleDevice.getLogicalDevice().mapMemory(triangleModel->getCubemapUniformBufferMemory(currentImage), 0, sizeof(skyboxObject.mesh->mvp));
                memcpy(data, &skyboxObject.mesh->mvp, sizeof(skyboxObject.mesh->mvp));
                triangleDevice.getLogicalDevice().unmapMemory(triangleModel->getCubemapUniformBufferMemory(currentImage));

                // Bind and draw
                if (lastPipeline != std::addressof(renderComponent->material.solidPipeline))
                    currentCommandBuffer.bindPipeline(
                        vk::PipelineBindPoint::eGraphics, 
                        (enableWireframe == false) ? renderComponent->material.solidPipeline : renderComponent->material.wireframePipeline);
                
                // Bind 3D object
                currentCommandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics, renderComponent->material.pipelineLayout, 0, triangleDescriptor->getDescriptorSet(currentFrame), dynamicOffset);

                triangleModel->bind(currentCommandBuffer,
                                    sizeof(renderComponent->mesh.vertices[0]) * renderComponent->mesh.vertices.size() * (entity.id - 1),
                                    sizeof(renderComponent->mesh.indices[0]) * renderComponent->mesh.indices.size() * (entity.id - 1));


                currentCommandBuffer.drawIndexed(static_cast<uint32_t>(renderComponent->mesh.indices.size()), 1, 0, 0, 0);

                // Bind skybox
                currentCommandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics, skyboxObject.material->pipelineLayout, 0, triangleDescriptor->getCubemapDescriptorSet(currentFrame), {0});

                triangleModel->bind(currentCommandBuffer, 0, 0);
                currentCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxObject.material->solidPipeline);

                currentCommandBuffer.drawIndexed(static_cast<uint32_t>(skyboxObject.mesh->indices.size()), 1, 0, 0, 0);

                dynamicOffset = entity.id * triangleModel->getDynamicAlignment();
                lastPipeline = std::addressof(renderComponent->material.solidPipeline);
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

        for (auto& entity : entities)
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