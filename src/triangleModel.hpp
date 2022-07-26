#pragma once

#include "triangleDevice.hpp"
#include "triangleTypes.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

namespace triangle
{
    class Model
    {
    public:
        Model(Device& device);
        ~Model();

        std::vector<vk::Buffer> getUniformBuffers() { return uniformBuffers; };
        vk::DeviceMemory getUniformBufferMemory(int index) { return uniformBufferMemories[index]; };
        vk::DeviceSize getDynamicAlignment() { return dynamicAlignment; }

        void bind(vk::CommandBuffer &commandBuffer, const vk::DeviceSize &vertexOffset, const vk::DeviceSize &indexOffset);
        void createUniformBuffers(const uint32_t bufferCount, const uint32_t entitySize);

        void allocVertexBuffer(const std::vector<std::vector<Vertex>>& a_Vertex);
        void allocIndexBuffer(const std::vector<std::vector<Index>>& a_Index);
        
    private:
        Device& device;

        uint32_t uniformBufferCount = 0;
        vk::DeviceSize dynamicAlignment = 0;

        vk::Buffer vertexBuffer = VK_NULL_HANDLE, indexBuffer = VK_NULL_HANDLE;
        vk::DeviceMemory vertexBufferMemory, indexBufferMemory;

        std::vector<vk::Buffer> uniformBuffers;
        std::vector<vk::DeviceMemory> uniformBufferMemories;

        void* data;

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    };
}