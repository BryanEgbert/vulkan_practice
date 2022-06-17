#pragma once

#include "triangleDevice.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <glm/glm.hpp>
#include <vector>

class TriangleModel
{
public:
    struct MVP
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct MeshPushConstant {
        alignas(16) glm::vec3 offset;
        alignas(16) glm::vec3 color;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;

        static std::vector<vk::VertexInputBindingDescription> getBindingDesciptions();

        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
    };

    TriangleModel(TriangleDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
    ~TriangleModel();

    std::vector<vk::Buffer> getUniformBuffers() { return uniformBuffers; };
    vk::DeviceMemory getUniformBufferMemory(int index) { return uniformBufferMemories[index]; };

    void bind(vk::CommandBuffer& commandBuffer);
    void createUniformBuffers(const uint32_t bufferCount);
    
private:
    TriangleDevice& device;

    uint32_t uniformBufferCount = 0;

    vk::Buffer vertexBuffer, indexBuffer;
    vk::DeviceMemory vertexBufferMemory, indexBufferMemory;

    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBufferMemories;

    void* data;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    void createVertexBuffer(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<uint16_t>& indices); 
};