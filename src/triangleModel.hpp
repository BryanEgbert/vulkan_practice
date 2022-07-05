#pragma once

#include "triangleDevice.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

using Index = uint16_t;

class TriangleModel
{
public:
    struct MVP
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;

        static std::vector<vk::VertexInputBindingDescription> getBindingDesciptions();

        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        MVP mvp;
        vk::Buffer vertexBuffer = VK_NULL_HANDLE, indexBuffer = VK_NULL_HANDLE;
        vk::DeviceMemory vertexBufferMemory, indexBufferMemory;

        Mesh(std::vector<Vertex> &a_Vertices, std::vector<Index> &a_Indices) : vertices{a_Vertices}, indices{a_Indices} {};
    };

    struct Transform
    {
        glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 rotation = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 scale = glm::vec3(0.f, 0.f, 0.f);
    };

    struct Material
    {
        vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
    };

    struct RenderModel
    {
        Mesh &mesh;
        Transform &transform;
        // Material &material;

        RenderModel(Mesh& a_Mesh, Transform& a_Transform) : mesh{a_Mesh}, transform{a_Transform}{};
    };


    struct MeshPushConstant {
        alignas(16) glm::vec3 offset;
        alignas(16) glm::vec3 color;
    };

    TriangleModel(TriangleDevice& device);
    ~TriangleModel();

    std::vector<vk::Buffer> getUniformBuffers() { return uniformBuffers; };
    vk::DeviceMemory getUniformBufferMemory(int index) { return uniformBufferMemories[index]; };

    void bind(vk::CommandBuffer &commandBuffer, Mesh& mesh);
    void createUniformBuffers(const uint32_t bufferCount);

    void allocVertexBuffer(Mesh &mesh);
    void allocIndexBuffer(Mesh &mesh);
    
private:
    TriangleDevice& device;

    uint32_t uniformBufferCount = 0;

    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBufferMemories;
    std::vector<RenderModel> m_RenderModels;

    void* data;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};