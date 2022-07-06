#include "triangleDescriptor.hpp"
#include "triangleModel.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <vector>

TriangleDescriptor::TriangleDescriptor(TriangleDevice& device, uint32_t descriptorCount, const std::vector<vk::Buffer>& buffers) : device{device}, descriptorCount{descriptorCount}
{
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets(buffers);
}

TriangleDescriptor::~TriangleDescriptor()
{
    device.getLogicalDevice().destroyDescriptorPool(descriptorPool);
    device.getLogicalDevice().destroyDescriptorSetLayout(descriptorSetLayout);
}

void TriangleDescriptor::createDescriptorPool()
{
    std::vector<vk::DescriptorPoolSize> poolSize;
    poolSize.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, descriptorCount));

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
        vk::DescriptorPoolCreateFlags(),
        descriptorCount,
        poolSize
    );

    descriptorPool = device.getLogicalDevice().createDescriptorPool(descriptorPoolCreateInfo);
}

void TriangleDescriptor::createDescriptorSetLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> descSetLayoutBindings;
    vk::DescriptorSetLayoutBinding cameraSetLayout = vk::DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex, nullptr
    );

    descSetLayoutBindings.push_back(cameraSetLayout);

    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo(
        vk::DescriptorSetLayoutCreateFlags(),
        descSetLayoutBindings
    );

    descriptorSetLayout = device.getLogicalDevice().createDescriptorSetLayout(layoutCreateInfo);
}


void TriangleDescriptor::createDescriptorSets(const std::vector<vk::Buffer>& buffers)
{
    std::vector<vk::DescriptorSetLayout> layouts(descriptorCount, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, layouts);

    descriptorSets.reserve(descriptorCount);

    descriptorSets = device.getLogicalDevice().allocateDescriptorSets(allocInfo);

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(descriptorCount);

    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(descriptorCount);

    for (int i = 0; i < descriptorCount; ++i)
    {
        bufferInfos.push_back(vk::DescriptorBufferInfo(
            buffers[i], 0, sizeof(TriangleModel::MVP)
        ));

        descriptorWrites.push_back(
            vk::WriteDescriptorSet(
                descriptorSets[i],
                0, 
                0, 
                vk::DescriptorType::eUniformBufferDynamic, 
                {}, 
                bufferInfos[i]
            )
        );

        device.getLogicalDevice().updateDescriptorSets(descriptorWrites[i], nullptr);
    }

}