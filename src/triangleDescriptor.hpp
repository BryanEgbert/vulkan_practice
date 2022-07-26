#pragma once

#include "triangleDevice.hpp"
#include "triangleSwapchain.hpp"
#include "triangleTypes.hpp"

#include <vulkan/vulkan.hpp>
#include <vector>

namespace triangle
{
    class Descriptor
    {
    public:
        Descriptor(Device &device, uint32_t descriptorCount, const std::vector<vk::Buffer> &buffers, const Swapchain::Texture &textureProperties);
        ~Descriptor();

        vk::DescriptorSet getDescriptorSet(uint32_t index) { return descriptorSets[index]; };
        vk::DescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; };

        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createDescriptorSets(const std::vector<vk::Buffer> &buffers, const Swapchain::Texture& textureProperties);

    private:
        Device& device;

        uint32_t descriptorCount;

        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;

        vk::DescriptorSetLayout descriptorSetLayout;
        vk::DescriptorSetLayoutBinding descSetLayoutBinding;

    };
}