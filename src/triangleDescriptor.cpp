#include "triangleDescriptor.hpp"
#include "triangleModel.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <vector>

namespace triangle
{
    Descriptor::Descriptor(Device &device, uint32_t descriptorCount, const std::vector<vk::Buffer> &buffers, const Swapchain::Texture &textureProperties)
        : device{device}, descriptorCount{descriptorCount}
    {
        createDescriptorSetLayout();
        createDescriptorPool();
        createDescriptorSets(buffers, textureProperties);
    }

    Descriptor::~Descriptor()
    {
        device.getLogicalDevice().destroyDescriptorPool(descriptorPool);
        device.getLogicalDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    }

    void Descriptor::createDescriptorPool()
    {
        std::vector<vk::DescriptorPoolSize> poolSize;
        poolSize.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, descriptorCount));
        poolSize.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, descriptorCount));

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
            vk::DescriptorPoolCreateFlags(),
            descriptorCount,
            poolSize
        );

        descriptorPool = device.getLogicalDevice().createDescriptorPool(descriptorPoolCreateInfo);
    }

    void Descriptor::createDescriptorSetLayout()
    {
        std::vector<vk::DescriptorSetLayoutBinding> descSetLayoutBindings;
        descSetLayoutBindings.reserve(descriptorCount);

        vk::DescriptorSetLayoutBinding cameraLayoutBinding = vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr
        );

        vk::DescriptorSetLayoutBinding samplerLayoutBinding = vk::DescriptorSetLayoutBinding(
            1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr
        );

        descSetLayoutBindings.push_back(cameraLayoutBinding);
        descSetLayoutBindings.push_back(samplerLayoutBinding);

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo(
            vk::DescriptorSetLayoutCreateFlags(),
            descSetLayoutBindings
        );

        descriptorSetLayout = device.getLogicalDevice().createDescriptorSetLayout(layoutCreateInfo);
    }

    void Descriptor::createDescriptorSets(const std::vector<vk::Buffer> &buffers, const Swapchain::Texture &textureProperties)
    {
        std::vector<vk::DescriptorSetLayout> layouts(descriptorCount, descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, layouts);

        descriptorSets.reserve(descriptorCount);

        descriptorSets = device.getLogicalDevice().allocateDescriptorSets(allocInfo);

        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(descriptorCount);

        std::vector<vk::DescriptorImageInfo> imageInfos;
        imageInfos.reserve(descriptorCount);

        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(descriptorCount * 2);

        for (int i = 0; i < descriptorCount; ++i)
        {
            // dynamic UBO
            bufferInfos.push_back(vk::DescriptorBufferInfo(
                buffers[i], 0, sizeof(Model::MVP)
            ));

            imageInfos.push_back(vk::DescriptorImageInfo(
                textureProperties.sampler, textureProperties.imageView, textureProperties.imageLayout 
            ));

            // Binding 0: Vertex shader dynamic UBO
            descriptorWrites.push_back(vk::WriteDescriptorSet(
                descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBufferDynamic, {}, bufferInfos
            ));

            // Binding 1: texture
            descriptorWrites.push_back( vk::WriteDescriptorSet(
                descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, imageInfos
            ));

            device.getLogicalDevice().updateDescriptorSets(descriptorWrites[i], nullptr);
        }

    }
}