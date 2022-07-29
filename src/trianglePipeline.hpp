#pragma once

#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "triangleSwapchain.hpp"

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include <optional>

namespace triangle
{
    static uint32_t modulesCreated = 0;
    class Pipeline
    {
    public:
        struct PipelineConfig
        {
            const std::vector<vk::VertexInputBindingDescription>& bindingDescriptions{};
            const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions{};
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
            vk::PipelineViewportStateCreateInfo viewportStateCreateInfo;
            vk::PipelineRasterizationStateCreateInfo rasterizerStateCreateInfo;
            vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
            std::optional<vk::PipelineDepthStencilStateCreateInfo> depthStenciStateCreateInfo;
            vk::PipelineColorBlendStateCreateInfo colorBlendingStateCreateInfo;
            std::optional<vk::PipelineDynamicStateCreateInfo> dynamicStateCreateInfo;
            vk::PipelineLayout pipelineLayout;
            vk::RenderPass renderPass;
        };

        Pipeline(Device &device);
        vk::Pipeline createGraphicsPipeline(PipelineConfig &pipelineConfig, const char *vertFilePath, const char *fragFilePath);
        vk::Pipeline createDefaultGraphicsPipeline(vk::PipelineLayout& layout, const vk::RenderPass& renderPass);
        vk::Pipeline createTextureGraphicsPipeline(vk::PipelineLayout &layout, const vk::RenderPass &renderPass);
        ~Pipeline();

        void bind(vk::CommandBuffer &commandBuffer);
    private:

        Device& device;

        static std::vector<char> readFile(const char* filename);

        std::vector<vk::ShaderModule> vertShaderModule, fragShaderModule;

        vk::ShaderModule createShaderModule(const std::vector<char>& code);
        vk::Pipeline pipeline;
    };
}