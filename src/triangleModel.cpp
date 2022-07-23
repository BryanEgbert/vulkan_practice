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

TriangleModel::TriangleModel(TriangleDevice& device) : device{device} {}

TriangleModel::~TriangleModel()
{
    device.getLogicalDevice().destroyBuffer(vertexBuffer);
    device.getLogicalDevice().freeMemory(vertexBufferMemory);

    device.getLogicalDevice().destroyBuffer(indexBuffer);
    device.getLogicalDevice().freeMemory(indexBufferMemory);

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
    attributeDescriptions.reserve(3);

    // position
    vk::VertexInputAttributeDescription attributeDescription(
        0,
        0,
        vk::Format::eR32G32B32Sfloat,
        offsetof(Vertex, pos)
    );

    attributeDescriptions.push_back(attributeDescription);

    // color
    attributeDescription.setLocation(1);
    attributeDescription.setOffset(offsetof(Vertex, color));
    attributeDescriptions.push_back(attributeDescription);

    // uv
    attributeDescription.setLocation(2);
    attributeDescription.setFormat(vk::Format::eR32G32Sfloat);
    attributeDescription.setOffset(offsetof(Vertex, uv));
    attributeDescriptions.push_back(attributeDescription);


    return attributeDescriptions;
}

void TriangleModel::allocVertexBuffer(const std::vector<std::vector<Vertex>>& a_Vertex)
{
    vk::DeviceSize bufferSize = 0, offset = 0;

    for (int i = 0; i < a_Vertex.size(); ++i)
    {
        bufferSize += sizeof(a_Vertex[0][0]) * a_Vertex[i].size();
    }

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    vk::MemoryPropertyFlags stagingBufferProperties(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, stagingBufferProperties, stagingBuffer, stagingBufferMemory);

    for (int i = 0; i < a_Vertex.size(); ++i)
    {
        int verticesByteSize = sizeof(a_Vertex[0][0]) * a_Vertex[i].size();

        data = device.getLogicalDevice().mapMemory(stagingBufferMemory, offset, verticesByteSize);
        memcpy(data, a_Vertex[i].data(), verticesByteSize);
        device.getLogicalDevice().unmapMemory(stagingBufferMemory);

        offset += verticesByteSize;
    }

    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

    device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    device.getLogicalDevice().destroyBuffer(stagingBuffer);
    device.getLogicalDevice().freeMemory(stagingBufferMemory);
}

void TriangleModel::allocIndexBuffer(const std::vector<std::vector<Index>> &a_Index)
{
    vk::DeviceSize bufferSize = 0, offset = 0;

    for (int i = 0; i < a_Index.size(); ++i)
    {
        bufferSize += sizeof(Index) * a_Index[i].size();
    }

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    for (int i = 0; i < a_Index.size(); ++i)
    {
        int indicesByteSize = sizeof(Index) * a_Index[i].size();

        data = device.getLogicalDevice().mapMemory(stagingBufferMemory, offset, indicesByteSize);
        memcpy(data, a_Index[i].data(), indicesByteSize);
        device.getLogicalDevice().unmapMemory(stagingBufferMemory);

        offset += indicesByteSize;
    }

    device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);
    device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    device.getLogicalDevice().destroyBuffer(stagingBuffer);
    device.getLogicalDevice().freeMemory(stagingBufferMemory);
}

void TriangleModel::createUniformBuffers(const uint32_t bufferCount, const uint32_t entitySize)
{
    uniformBufferCount = bufferCount;

    vk::DeviceSize minUboAlignment = device.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
    dynamicAlignment = sizeof(MVP);

    if (minUboAlignment > 0)
        dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) &  ~(minUboAlignment - 1);

    vk::DeviceSize bufferSize = dynamicAlignment * entitySize;

    uniformBuffers.resize(bufferCount);
    uniformBufferMemories.resize(bufferCount);

    vk::MemoryPropertyFlags memoryProperty(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    for (int i = 0; i < bufferCount; ++i)
        device.createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, memoryProperty, uniformBuffers[i], uniformBufferMemories[i]);
}

void TriangleModel::bind(vk::CommandBuffer &commandBuffer, const vk::DeviceSize &vertexOffset, const vk::DeviceSize &indexOffset)
{
    commandBuffer.bindVertexBuffers(0, vertexBuffer, vertexOffset);
    commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);
}
