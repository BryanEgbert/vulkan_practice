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

namespace triangle
{
    Pipeline::Pipeline(Device &device)
        : device{device} {}

    Pipeline::~Pipeline()
    {
        for (auto& shaderModule : vertShaderModule)
            device.getLogicalDevice().destroyShaderModule(shaderModule);

        for (auto &shaderModule : fragShaderModule)
            device.getLogicalDevice().destroyShaderModule(shaderModule);

    }

    std::vector<char> Pipeline::readFile(const char* filename)
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

    void Pipeline::bind(vk::CommandBuffer &commandBuffer)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

    vk::Pipeline Pipeline::createGraphicsPipeline(PipelineConfig &pipelineConfig, const char *vertFilePath, const char *fragFilePath)
    {
        auto vertShaderCode = Pipeline::readFile(vertFilePath);
        auto fragShaderCode = Pipeline::readFile(fragFilePath);

        vertShaderModule.push_back(createShaderModule(vertShaderCode));
        fragShaderModule.push_back(createShaderModule(fragShaderCode));

        std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
            vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertShaderModule[modulesCreated], "main", nullptr),
            vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragShaderModule[modulesCreated], "main", nullptr)};

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
        std::tie(result, pipeline) = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);
        modulesCreated++;

        return pipeline;
    }

    vk::Pipeline Pipeline::createDefaultGraphicsPipeline(vk::PipelineLayout &layout, const vk::RenderPass &renderPass)
    {
        vk::Viewport viewport(0.0f, 0.0f, 0.f, 0.f, 0.0f, 1.0f);
        vk::Rect2D scissor({0, 0}, {0, 0});

        vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR |
                                                    vk::ColorComponentFlagBits::eG |
                                                    vk::ColorComponentFlagBits::eB |
                                                    vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            colorComponentFlags);

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        Pipeline::PipelineConfig pipelineConfig{
            Vertex::getBindingDesciptions(),
            Vertex::getAttributeDescriptions(),
            vk::PipelineInputAssemblyStateCreateInfo(
                vk::PipelineInputAssemblyStateCreateFlags(),
                vk::PrimitiveTopology::eTriangleList, false),
            vk::PipelineViewportStateCreateInfo(
                vk::PipelineViewportStateCreateFlags(), viewport, scissor),
            vk::PipelineRasterizationStateCreateInfo(
                vk::PipelineRasterizationStateCreateFlags(),
                false,
                false,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eNone,
                vk::FrontFace::eCounterClockwise,
                false,
                0.0f,
                0.0f,
                0.0f,
                1.0f),
            vk::PipelineMultisampleStateCreateInfo(
                vk::PipelineMultisampleStateCreateFlags(),
                vk::SampleCountFlagBits::e1,
                false,
                1.0f,
                nullptr,
                false,
                false),
            vk::PipelineDepthStencilStateCreateInfo(
                vk::PipelineDepthStencilStateCreateFlags(),
                true,
                true,
                vk::CompareOp::eLess,
                false,
                false),
            vk::PipelineColorBlendStateCreateInfo(
                vk::PipelineColorBlendStateCreateFlags(),
                false,
                vk::LogicOp::eCopy,
                colorBlendAttachmentState,
                {{1.0f, 1.0f, 1.0f, 1.0f}}),
            vk::PipelineDynamicStateCreateInfo(
                vk::PipelineDynamicStateCreateFlags(),
                dynamicStates),
            layout,
            renderPass};

        auto vertShaderCode = Pipeline::readFile("../shaders/spv/defaultVert.spv");
        auto fragShaderCode = Pipeline::readFile("../shaders/spv/defaultFrag.spv");

        vertShaderModule.push_back(createShaderModule(vertShaderCode));
        fragShaderModule.push_back(createShaderModule(fragShaderCode));

        std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
            vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertShaderModule[modulesCreated], "main", nullptr),
            vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragShaderModule[modulesCreated], "main", nullptr)};

        vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo(
            vk::PipelineVertexInputStateCreateFlags(),
            pipelineConfig.bindingDescriptions,
            pipelineConfig.attributeDescriptions);

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
            pipelineConfig.renderPass);

        vk::Result result;
        std::tie(result, pipeline) = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);

        modulesCreated++;

        return pipeline;
    }

    vk::Pipeline Pipeline::createTextureGraphicsPipeline(vk::PipelineLayout &layout, const vk::RenderPass &renderPass)
    {
        vk::Viewport viewport(0.0f, 0.0f, 0.f, 0.f, 0.0f, 1.0f);
        vk::Rect2D scissor({0, 0}, {0, 0});

        vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR |
                                                    vk::ColorComponentFlagBits::eG |
                                                    vk::ColorComponentFlagBits::eB |
                                                    vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            colorComponentFlags);

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        Pipeline::PipelineConfig pipelineConfig{
            Vertex::getBindingDesciptions(),
            Vertex::getAttributeDescriptions(),
            vk::PipelineInputAssemblyStateCreateInfo(
                vk::PipelineInputAssemblyStateCreateFlags(),
                vk::PrimitiveTopology::eTriangleList, false),
            vk::PipelineViewportStateCreateInfo(
                vk::PipelineViewportStateCreateFlags(), viewport, scissor),
            vk::PipelineRasterizationStateCreateInfo(
                vk::PipelineRasterizationStateCreateFlags(),
                false,
                false,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eNone,
                vk::FrontFace::eCounterClockwise,
                false,
                0.0f,
                0.0f,
                0.0f,
                1.0f),
            vk::PipelineMultisampleStateCreateInfo(
                vk::PipelineMultisampleStateCreateFlags(),
                vk::SampleCountFlagBits::e1,
                false,
                1.0f,
                nullptr,
                false,
                false),
            vk::PipelineDepthStencilStateCreateInfo(
                vk::PipelineDepthStencilStateCreateFlags(),
                true,
                true,
                vk::CompareOp::eLess,
                false,
                false),
            vk::PipelineColorBlendStateCreateInfo(
                vk::PipelineColorBlendStateCreateFlags(),
                false,
                vk::LogicOp::eCopy,
                colorBlendAttachmentState,
                {{1.0f, 1.0f, 1.0f, 1.0f}}),
            vk::PipelineDynamicStateCreateInfo(
                vk::PipelineDynamicStateCreateFlags(),
                dynamicStates),
            layout,
            renderPass};

        auto vertShaderCode = Pipeline::readFile("../shaders/spv/texturedVert.spv");
        auto fragShaderCode = Pipeline::readFile("../shaders/spv/texturedFrag.spv");

        vertShaderModule.push_back(createShaderModule(vertShaderCode));
        fragShaderModule.push_back(createShaderModule(fragShaderCode));

        std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
            vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertShaderModule[modulesCreated], "main", nullptr),
            vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragShaderModule[modulesCreated], "main", nullptr)};

        vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo(
            vk::PipelineVertexInputStateCreateFlags(),
            pipelineConfig.bindingDescriptions,
            pipelineConfig.attributeDescriptions);

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
            pipelineConfig.renderPass);

        vk::Result result;
        std::tie(result, pipeline) = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);

        modulesCreated++;

        return pipeline;
    }

    vk::ShaderModule Pipeline::createShaderModule(const std::vector<char>& code)
    {
        vk::ShaderModuleCreateInfo shaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), code.size(), reinterpret_cast<const uint32_t*>(code.data()));

        vk::ShaderModule shaderModule = device.getLogicalDevice().createShaderModule(shaderModuleCreateInfo);

        return shaderModule;
    }
}
