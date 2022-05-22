#include "triangleSwapchain.hpp"
#include "triangleDevice.hpp"
#include "triangleWindow.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

TriangleSwapchain::TriangleSwapchain(TriangleDevice& device) : device{device}
{
    querySwapchainSupport();
    initSurfaceProperties();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createUIRenderPass();
    createDepthResources();
    createFrameBuffers();
    createSyncObject();
}

TriangleSwapchain::~TriangleSwapchain()
{
    device.getLogicalDevice().destroyImageView(depthImageView);
    device.getLogicalDevice().destroyImage(depthImage);
    device.getLogicalDevice().freeMemory(depthImageMemory);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        device.getLogicalDevice().destroySemaphore(imageAvailableSemaphore[i]);
        device.getLogicalDevice().destroySemaphore(renderFinishedSemaphore[i]);
        device.getLogicalDevice().destroyFence(inFlightFences[i]);
    }


    std::cout << "Destroying framebuffers...\n";
    for (const auto& framebuffer : framebuffers)
        device.getLogicalDevice().destroyFramebuffer(framebuffer);

    std::cout << "Destroying render pass...\n";
    device.getLogicalDevice().destroyRenderPass(mainRenderPass);

    std::cout << "Destroying image views...\n";
    for (auto& imageView : imageViews)
    {
        device.getLogicalDevice().destroyImageView(imageView);
    }

    std::cout << "Destroying swapchain...\n";
    device.getLogicalDevice().destroySwapchainKHR(swapchain);
}

void TriangleSwapchain::querySwapchainSupport()
{  
    std::vector<vk::ExtensionProperties> extensionProperties = device.getPhysicalDevice().enumerateDeviceExtensionProperties();
    for (const auto& prop : extensionProperties)
    {
        if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, prop.extensionName) == 0)
            return;
    }

    std::cout << "Swapchain is not supported in your machine\n";
    exit(1);
}

void TriangleSwapchain::initSurfaceProperties()
{
    swapChainSupportDetails.capabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(device.getSurface());
    std::cout << "Capabilities:\n";
    std::cout << "\t"
              << "currentExtent           = " << swapChainSupportDetails.capabilities.currentExtent.width << " x " << swapChainSupportDetails.capabilities.currentExtent.height << "\n";
    std::cout << "\t"
              << "currentTransform        = " << vk::to_string( swapChainSupportDetails.capabilities.currentTransform ) << "\n";
    std::cout << "\t"
              << "maxImageArrayLayers     = " << swapChainSupportDetails.capabilities.maxImageArrayLayers << "\n";
    std::cout << "\t"
              << "maxImageCount           = " << swapChainSupportDetails.capabilities.maxImageCount << "\n";
    std::cout << "\t"
              << "maxImageExtent          = " << swapChainSupportDetails.capabilities.maxImageExtent.width << " x " << swapChainSupportDetails.capabilities.maxImageExtent.height << "\n";
    std::cout << "\t"
              << "minImageCount           = " << swapChainSupportDetails.capabilities.minImageCount << "\n";
    std::cout << "\t"
              << "minImageExtent          = " << swapChainSupportDetails.capabilities.minImageExtent.width << " x " << swapChainSupportDetails.capabilities.minImageExtent.height << "\n";
    std::cout << "\t"
              << "supportedCompositeAlpha = " << vk::to_string( swapChainSupportDetails.capabilities.supportedCompositeAlpha ) << "\n";
    std::cout << "\t"
              << "supportedTransforms     = " << vk::to_string( swapChainSupportDetails.capabilities.supportedTransforms ) << "\n";
    std::cout << "\t"
              << "supportedUsageFlags     = " << vk::to_string( swapChainSupportDetails.capabilities.supportedUsageFlags ) << "\n";
    std::cout << "\n";

    uint32_t width, height;
    if (swapChainSupportDetails.capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        swapchainExtent.setWidth(std::clamp(width, swapChainSupportDetails.capabilities.minImageExtent.width, swapChainSupportDetails.capabilities.maxImageExtent.width));
        swapchainExtent.setHeight(std::clamp(height, swapChainSupportDetails.capabilities.minImageExtent.height, swapChainSupportDetails.capabilities.maxImageExtent.height));
    }
    else
    {
       swapchainExtent = swapChainSupportDetails.capabilities.currentExtent;
    }

    swapChainSupportDetails.formats = device.getPhysicalDevice().getSurfaceFormatsKHR(device.getSurface());

    std::cout << "Formats:\n";
    for ( size_t j = 0; j < swapChainSupportDetails.formats.size(); j++ )
    {
        std::cout << "\tFormat " << j << "\n";
        std::cout << "\t\t"
                    << "colorSpace  = " << vk::to_string( swapChainSupportDetails.formats[j].colorSpace ) << "\n";
        std::cout << "\t\t"
                    << "format      = " << vk::to_string( swapChainSupportDetails.formats[j].format ) << "\n";
        std::cout << "\n";
    }

    format = swapChainSupportDetails.formats[0];
    for (const auto& surfaceFormat : swapChainSupportDetails.formats)
    {
        if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            format = surfaceFormat;
            break;
        }
    }

    swapChainSupportDetails.presentModes = device.getPhysicalDevice().getSurfacePresentModesKHR(device.getSurface());
    for ( size_t j = 0; j < swapChainSupportDetails.formats.size(); j++ )
    {
        std::cout << "\tPresent mode " << j << ":\t"
                  << vk::to_string( swapChainSupportDetails.presentModes[j] ) << '\n';
    }

    presentMode = vk::PresentModeKHR::eFifo;
}

vk::Format TriangleSwapchain::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates)
    {
        vk::FormatProperties props = device.getPhysicalDevice().getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
            return format;

    }

    throw std::runtime_error("Failed to find supported format");
}

vk::Format TriangleSwapchain::findDepthFormat()
{
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, 
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}



void TriangleSwapchain::createSwapchain()
{
    auto queueFamilyIndex = device.getQueueFamilyIndex();

    vk::SwapchainCreateInfoKHR swapchainCreateInfo( vk::SwapchainCreateFlagsKHR(), 
                                                    device.getSurface(), 
                                                    swapChainSupportDetails.capabilities.minImageCount + 1, 
                                                    format.format, 
                                                    format.colorSpace, 
                                                    swapchainExtent, 
                                                    1, 
                                                    vk::ImageUsageFlagBits::eColorAttachment, 
                                                    vk::SharingMode::eExclusive, 
                                                    {},
                                                    swapChainSupportDetails.capabilities.currentTransform,
                                                    vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                    presentMode,
                                                    true,
                                                    nullptr);

    if (queueFamilyIndex.graphics != queueFamilyIndex.present)
    {
        uint32_t queueFamilyIndices[2] = {queueFamilyIndex.graphics, queueFamilyIndex.present};

        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapchainCreateInfo.setQueueFamilyIndexCount(2);
        swapchainCreateInfo.setPQueueFamilyIndices(queueFamilyIndices);
    }

    swapchain = device.getLogicalDevice().createSwapchainKHR(swapchainCreateInfo);
}

void TriangleSwapchain::createImageViews()
{
    std::vector<vk::Image> swapchainImages = device.getLogicalDevice().getSwapchainImagesKHR(swapchain);

    imageViews.reserve(swapchainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo(
        {}, 
        {}, 
        vk::ImageViewType::e2D, 
        format.format, 
        {}, 
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        
    for (auto image : swapchainImages)
    {
        imageViewCreateInfo.setImage(image);
        imageViews.push_back(device.getLogicalDevice().createImageView(imageViewCreateInfo));
    }
}

void TriangleSwapchain::createDepthResources()
{
    vk::Format depthFormat = findDepthFormat();

    createImage(depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
    createImageView(depthImage, depthImageView, depthFormat, vk::ImageAspectFlagBits::eDepth);

}

void TriangleSwapchain::createRenderPass()
{
    std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
    attachmentDescriptions[0] = vk::AttachmentDescription( vk::AttachmentDescriptionFlags(), 
                                                            format.format, 
                                                            vk::SampleCountFlagBits::e1,
                                                            vk::AttachmentLoadOp::eClear,
                                                            vk::AttachmentStoreOp::eStore, 
                                                            vk::AttachmentLoadOp::eDontCare,
                                                            vk::AttachmentStoreOp::eDontCare,
                                                            vk::ImageLayout::eUndefined, 
                                                            vk::ImageLayout::ePresentSrcKHR);

    attachmentDescriptions[1] = vk::AttachmentDescription(  vk::AttachmentDescriptionFlags(), 
                                                            findDepthFormat(), 
                                                            vk::SampleCountFlagBits::e1, 
                                                            vk::AttachmentLoadOp::eClear, 
                                                            vk::AttachmentStoreOp::eDontCare, 
                                                            vk::AttachmentLoadOp::eDontCare, 
                                                            vk::AttachmentStoreOp::eDontCare, 
                                                            vk::ImageLayout::eUndefined, 
                                                            vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass(
        vk::SubpassDescriptionFlags(), 
        vk::PipelineBindPoint::eGraphics, 
        {}, 
        colorReference, 
        nullptr, 
        &depthAttachmentRef, 
        nullptr);

    vk::SubpassDependency subpassDependency(
        VK_SUBPASS_EXTERNAL, 
        0, 
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, 
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
    );

    vk::RenderPassCreateInfo renderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpass, subpassDependency);
    mainRenderPass = device.getLogicalDevice().createRenderPass(renderPassCreateInfo);
}

void TriangleSwapchain::createUIRenderPass()
{
    std::array<vk::AttachmentDescription, 1> attachmentDescriptions;
    attachmentDescriptions[0] = vk::AttachmentDescription(
        vk::AttachmentDescriptionFlags(),
        format.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass(
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        nullptr,
        colorReference
    );

    vk::SubpassDependency dependency(
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    vk::RenderPassCreateInfo renderPassCreateInfo(
        vk::RenderPassCreateFlags(),
        attachmentDescriptions,
        subpass,
        dependency
    );

    uiRenderPass = device.getLogicalDevice().createRenderPass(renderPassCreateInfo);
}

void TriangleSwapchain::createFrameBuffers()
{
    std::array<vk::ImageView, 2> attachments;

    vk::FramebufferCreateInfo framebufferCreateInfo(
        vk::FramebufferCreateFlags(), 
        mainRenderPass, 
        attachments, 
        swapchainExtent.width, 
        swapchainExtent.height, 
        1);
    
    framebuffers.reserve(imageViews.size());
    for (const auto& imageView : imageViews)
    {
        attachments[0] = imageView;
        attachments[1] = depthImageView;
        framebuffers.push_back(device.getLogicalDevice().createFramebuffer(framebufferCreateInfo));
    }
}

void TriangleSwapchain::createSyncObject()
{
    imageAvailableSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreCreateInfo(vk::SemaphoreCreateFlags(), nullptr);
    vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled, nullptr);

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        imageAvailableSemaphore.push_back(device.getLogicalDevice().createSemaphore(semaphoreCreateInfo));
        renderFinishedSemaphore.push_back(device.getLogicalDevice().createSemaphore(semaphoreCreateInfo));

        inFlightFences.push_back(device.getLogicalDevice().createFence(fenceCreateInfo));
    }

}

vk::Result TriangleSwapchain::acquireNextImage(uint32_t* imageIndex, uint32_t& currentFrame)
{
    if (device.getLogicalDevice().waitForFences(inFlightFences[currentFrame], true, UINT64_MAX) != vk::Result::eSuccess)
        throw std::runtime_error("Something's wrong when waiting for fences");

    device.getLogicalDevice().resetFences(inFlightFences[currentFrame]);

    return device.getLogicalDevice().acquireNextImageKHR(
        swapchain, 
        std::numeric_limits<uint64_t>::max(), 
        imageAvailableSemaphore[currentFrame], 
        nullptr, 
        imageIndex);
}

void TriangleSwapchain::createImage(vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
{
    vk::ImageCreateInfo imageCreateInfo(
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        format, 
        {swapchainExtent.width, swapchainExtent.height, 1},
        1, 
        1, 
        vk::SampleCountFlagBits::e1, 
        tiling, 
        usage
    );

    image = device.getLogicalDevice().createImage(imageCreateInfo);

    device.allocateAndBindImage(depthImageMemory, depthImage, properties);
}

void TriangleSwapchain::createImageView(vk::Image& image, vk::ImageView& imageView, vk::Format& format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo imageViewCreateInfo(
        vk::ImageViewCreateFlags(),
        image, 
        vk::ImageViewType::e2D, 
        format, 
        {},
        {aspectFlags, 0, 1, 0, 1}
    );

    imageView = device.getLogicalDevice().createImageView(imageViewCreateInfo);
}

