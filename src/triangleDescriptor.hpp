#pragma once

#include "triangleDevice.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"
#include <vector>


class TriangleDescriptor
{
public:
    TriangleDescriptor(TriangleDevice& device, uint32_t descriptorCount, const std::vector<vk::Buffer>& buffers);
    ~TriangleDescriptor();

    vk::DescriptorSet getDescriptorSet(uint32_t index) { return descriptorSets[index]; };
    vk::DescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; };

    void createDescriptorPool();
    void createDescriptorSetLayout();
    void createDescriptorSets(const std::vector<vk::Buffer>& buffers);
private:
    TriangleDevice& device;

    uint32_t descriptorCount;

    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSetLayoutBinding descSetLayoutBinding;

};