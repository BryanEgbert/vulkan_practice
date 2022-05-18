#include "triangleModel.hpp"
#include "triangleDevice.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <array>
#include <cstring>
#include <stdexcept>
#include <tuple>
#include <cstddef>
#include <vector>

TriangleModel::TriangleModel(TriangleDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) : device{device}
{
    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}

TriangleModel::~TriangleModel()
{
    device.getLogicalDevice().destroyBuffer(indexBuffer);
    device.getLogicalDevice().freeMemory(indexBufferMemory);

    device.getLogicalDevice().destroyBuffer(vertexBuffer);
    device.getLogicalDevice().freeMemory(vertexBufferMemory);

    for (int i = 0; i < uniformBufferCount; ++i)
    {
        device.getLogicalDevice().destroyBuffer(uniformBuffers[i]);
        device.getLogicalDevice().freeMemory(uniformBufferMemories[i]);
    }
}

std::vector<vk::VertexInputBindingDescription> TriangleModel::Vertex::getBindingDesciptions()
{
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
    bindingDescriptions.reserve(1);

    vk::VertexInputBindingDescription bindingDescription(0, sizeof(Vertex));

    bindingDescriptions.push_back(bindingDescription);

    return bindingDescriptions;   
}

std::vector<vk::VertexInputAttributeDescription> TriangleModel::Vertex::getAttributeDescriptions()
{
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.reserve(2);

    vk::VertexInputAttributeDescription attributeDescription(
        0,
        0,
        vk::Format::eR32G32B32Sfloat,
        offsetof(Vertex, pos)
    );

    attributeDescriptions.push_back(attributeDescription);

    attributeDescription.setLocation(1);
    attributeDescription.setOffset(offsetof(Vertex, color));
    attributeDescriptions.push_back(attributeDescription);


    return attributeDescriptions;
}

void TriangleModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    vk::MemoryPropertyFlags stagingBufferProperties(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, stagingBufferProperties, stagingBuffer, stagingBufferMemory);

    data = device.getLogicalDevice().mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(data, vertices.data(), (size_t)bufferSize);
    device.getLogicalDevice().unmapMemory(stagingBufferMemory);

    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

    device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    device.getLogicalDevice().destroyBuffer(stagingBuffer);
    device.getLogicalDevice().freeMemory(stagingBufferMemory);
}

void TriangleModel::createIndexBuffer(const std::vector<uint16_t> &indices)
{
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    data = device.getLogicalDevice().mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, indices.data(), (size_t)bufferSize);
    device.getLogicalDevice().unmapMemory(stagingBufferMemory);

    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);
    device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    device.getLogicalDevice().destroyBuffer(stagingBuffer);
    device.getLogicalDevice().freeMemory(stagingBufferMemory);
}

void TriangleModel::createUniformBuffers(const uint32_t bufferCount)
{
    uniformBufferCount = bufferCount;

    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(bufferCount);
    uniformBufferMemories.resize(bufferCount);

    vk::MemoryPropertyFlags memoryProperty(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    for (int i = 0; i < bufferCount; ++i)
        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, memoryProperty, uniformBuffers[i], uniformBufferMemories[i]);
}

void TriangleModel::bind(vk::CommandBuffer& commandBuffer)
{
    commandBuffer.bindVertexBuffers(0, vertexBuffer, {0});
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
}
