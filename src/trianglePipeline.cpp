#include "trianglePipeline.hpp"
#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "triangleSwapchain.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

TrianglePipeline::TrianglePipeline(TriangleDevice &device, PipelineConfig &pipelineConfig, TriangleModel::Material &material, const char *vertFilePath, const char *fragFilePath)
    : device{device}, pipelineConfig{pipelineConfig}, vertShaderPath{vertFilePath}, fragShaderPath{fragFilePath}
{
    std::cout << "Creating pipeline\n";
    createGraphicsPipeline(material);
}

TrianglePipeline::~TrianglePipeline()
{
    device.getLogicalDevice().destroyShaderModule(fragShaderModule);
    device.getLogicalDevice().destroyShaderModule(vertShaderModule);
    device.getLogicalDevice().destroyPipeline(pipeline);
}

std::vector<char> TrianglePipeline::readFile(const char* filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error("Failed to open file");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void TrianglePipeline::bind(vk::CommandBuffer &commandBuffer, const vk::Pipeline& materialPipeline)
{
    std::cout << "Binding pipeline: " << materialPipeline << '\n';
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, materialPipeline);
}

void TrianglePipeline::createGraphicsPipeline(TriangleModel::Material &material)
{
    auto vertShaderCode = TrianglePipeline::readFile(vertShaderPath);
    auto fragShaderCode = TrianglePipeline::readFile(fragShaderPath);

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main", nullptr),    
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment,fragShaderModule,"main", nullptr)
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo(
        vk::PipelineVertexInputStateCreateFlags(),
        pipelineConfig.bindingDescriptions,
        pipelineConfig.attributeDescriptions
    );

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
        vk::PipelineCreateFlags(),
        pipelineShaderStageCreateInfos,
        &vertexInputCreateInfo,
        &pipelineConfig.inputAssemblyCreateInfo,
        nullptr,
        &pipelineConfig.viewportStateCreateInfo,
        &pipelineConfig.rasterizerStateCreateInfo,
        &pipelineConfig.multisampleStateCreateInfo,
        pipelineConfig.depthStenciStateCreateInfo ? &pipelineConfig.depthStenciStateCreateInfo.value() : nullptr,
        &pipelineConfig.colorBlendingStateCreateInfo,
        pipelineConfig.dynamicStateCreateInfo ? &pipelineConfig.dynamicStateCreateInfo.value() : nullptr,
        pipelineConfig.pipelineLayout,
        pipelineConfig.renderPass
    );

    vk::Result result;
    std::tie(result, material.pipeline) = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);
}

vk::ShaderModule TrianglePipeline::createShaderModule(const std::vector<char>& code)
{
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), code.size(), reinterpret_cast<const uint32_t*>(code.data()));

    vk::ShaderModule shaderModule = device.getLogicalDevice().createShaderModule(shaderModuleCreateInfo);

    return shaderModule;
}
