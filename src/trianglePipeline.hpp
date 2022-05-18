#pragma once

#include "triangleDevice.hpp"
#include "triangleModel.hpp"
#include "triangleSwapchain.hpp"

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include <optional>

class TrianglePipeline
{
public:
    struct PipelineConfig
    {
        const std::vector<vk::VertexInputBindingDescription>& bindingDescriptions;
        const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions;
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

    TrianglePipeline(TriangleDevice& device, PipelineConfig& pipelineConfig, const char* vertFilePath, const char* fragFilePath);
    ~TrianglePipeline();

    void bind(vk::CommandBuffer& commandBuffer);
private:

    TriangleDevice& device;
    PipelineConfig pipelineConfig;
    const char* vertShaderPath;
    const char* fragShaderPath;

    static std::vector<char> readFile(const char* filename);

    vk::ShaderModule vertShaderModule, fragShaderModule;
    vk::Pipeline pipeline;

    vk::ShaderModule createShaderModule(const std::vector<char>& code);
    void createGraphicsPipeline();
};