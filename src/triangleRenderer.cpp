// #include "triangleRenderer.hpp"
// #include "triangleDevice.hpp"
// #include "trianglePipeline.hpp"
// #include "triangleSwapchain.hpp"
// #include "vulkan/vulkan_core.h"
// #include "vulkan/vulkan_enums.hpp"
// #include "vulkan/vulkan_handles.hpp"
// #include "vulkan/vulkan_structs.hpp"
// #include <array>
// #include <cstdint>

// TriangleRenderer::TriangleRenderer(TriangleDevice& device, TriangleSwapchain& swapchain) 
//     : device{device}, swapchain{swapchain}
// {
// 	createPipelineLayout();
// 	createPipeline();
//     createCommandBuffer();
// }

// TriangleRenderer::~TriangleRenderer()
// {
    

//     // device.getLogicalDevice().freeCommandBuffers(device.getCommandPool(), commandBuffer);
// }

// void TriangleRenderer::createCommandBuffer()
// {
//     commandBuffers.resize(swapchain.MAX_FRAMES_IN_FLIGHT);

//     vk::CommandBufferAllocateInfo cmdBufferAllocateInfo(
//         device.getCommandPool(),
//         vk::CommandBufferLevel::ePrimary,
//         commandBuffers.size());

//     commandBuffers = device.getLogicalDevice().allocateCommandBuffers(cmdBufferAllocateInfo);
// }

// void TriangleRenderer::createPipeline()
// {
//     vk::Viewport viewport(0.0f, 0.0f, (float)swapchain.getExtent().width, (float)swapchain.getExtent().height, 0.0f, 1.0f);
//     vk::Rect2D scissor({0, 0}, swapchain.getExtent());

//     vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR |
//                                                 vk::ColorComponentFlagBits::eG |
//                                                 vk::ColorComponentFlagBits::eB |
//                                                 vk::ColorComponentFlagBits::eA);
//     vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
//         false,
//         vk::BlendFactor::eOne,
//         vk::BlendFactor::eZero,
//         vk::BlendOp::eAdd,
//         vk::BlendFactor::eOne,
//         vk::BlendFactor::eZero,
//         vk::BlendOp::eAdd,
//         colorComponentFlags);

//     TrianglePipeline::PipelineConfig pipelineConfig{
//         TriangleModel::Vertex::getBindingDesciptions(),
//         TriangleModel::Vertex::getAttributeDescriptions(),
//         vk::PipelineInputAssemblyStateCreateInfo(
//             vk::PipelineInputAssemblyStateCreateFlags(),
//             vk::PrimitiveTopology::eTriangleList, false),
//         vk::PipelineViewportStateCreateInfo(
//             vk::PipelineViewportStateCreateFlags(), viewport, scissor),
//         vk::PipelineRasterizationStateCreateInfo(
//             vk::PipelineRasterizationStateCreateFlags(),
//             false,
//             false,
//             vk::PolygonMode::eFill,
//             vk::CullModeFlagBits::eNone,
//             vk::FrontFace::eCounterClockwise,
//             false,
//             0.0f,
//             0.0f,
//             0.0f,
//             1.0f),
//         vk::PipelineMultisampleStateCreateInfo(
//             vk::PipelineMultisampleStateCreateFlags(),
//             vk::SampleCountFlagBits::e1,
//             false,
//             1.0f,
//             nullptr,
//             false,
//             false),
//         vk::PipelineDepthStencilStateCreateInfo(
//             vk::PipelineDepthStencilStateCreateFlags(),
//             true,
//             true,
//             vk::CompareOp::eLess,
//             false,
//             false),
//         vk::PipelineColorBlendStateCreateInfo(
//             vk::PipelineColorBlendStateCreateFlags(),
//             false,
//             vk::LogicOp::eCopy,
//             colorBlendAttachmentState,
//             {{1.0f, 1.0f, 1.0f, 1.0f}}),
//         std::nullopt,
//         pipelineLayout,
//         swapchain.getMainRenderPass()};

//     trianglePipeline = std::make_unique<TrianglePipeline>(device, pipelineConfig, "shaders/vert.spv", "shaders/frag.spv");
// }

// void TriangleRenderer::createPipelineLayout()
// {

// }

// void TriangleRenderer::drawFrames()
// {
    
// }