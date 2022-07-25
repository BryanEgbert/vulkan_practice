#include "triangleSwapchain.hpp"

#include <imgui/imgui_impl_vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <fstream>

namespace triangle
{    
    Swapchain::Swapchain(Device& device) : device{device}
    {
        querySwapchainSupport();
        initSurfaceProperties();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createUIRenderPass();
        createDepthResources();
        createFrameBuffers();
        loadTexture("../textures/sample.ktx");
        createSyncObject();
    }

    Swapchain::~Swapchain()
    {
        device.getLogicalDevice().destroySampler(textureProperties.sampler);

        device.getLogicalDevice().destroyImage(textureProperties.image);
        device.getLogicalDevice().destroyImageView(textureProperties.imageView);
        device.getLogicalDevice().freeMemory(textureProperties.deviceMemory);

        device.getLogicalDevice().destroyImage(depthImage);
        device.getLogicalDevice().destroyImageView(depthImageView);
        device.getLogicalDevice().freeMemory(depthImageMemory);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            device.getLogicalDevice().destroySemaphore(imageAvailableSemaphore[i]);
            device.getLogicalDevice().destroySemaphore(renderFinishedSemaphore[i]);
            device.getLogicalDevice().destroyFence(inFlightFences[i]);
        }

        for (int i = 0; i < imageViews.size(); ++i)
        {
            device.getLogicalDevice().destroyFramebuffer(mainFrameBuffers[i]);
            device.getLogicalDevice().destroyFramebuffer(uiFrameBuffers[i]);
        }

        device.getLogicalDevice().destroyRenderPass(mainRenderPass);
        device.getLogicalDevice().destroyRenderPass(uiRenderPass);

        for (auto &imageView : imageViews)
        {
            device.getLogicalDevice().destroyImageView(imageView);
        }

        device.getLogicalDevice().destroySwapchainKHR(swapchain);
    }

    void Swapchain::querySwapchainSupport()
    {
        std::vector<vk::ExtensionProperties> extensionProperties = device.getPhysicalDevice().enumerateDeviceExtensionProperties();
        for (const auto &prop : extensionProperties)
        {
            if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, prop.extensionName) == 0)
                return;
        }

        std::cout << "Swapchain is not supported in your machine\n";
        exit(1);
    }

    void Swapchain::initSurfaceProperties()
    {
        swapChainSupportDetails.capabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(device.getSurface());

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

        presentMode = vk::PresentModeKHR::eFifo;
    }

    vk::Format Swapchain::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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

    vk::Format Swapchain::findDepthFormat()
    {
        return findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, 
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }



    void Swapchain::createSwapchain()
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

    void Swapchain::createImageViews()
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

    void Swapchain::createDepthResources()
    {
        vk::Format depthFormat = findDepthFormat();

        vk::ImageCreateInfo imageCreateInfo(
            vk::ImageCreateFlags(),
            vk::ImageType::e2D,
            depthFormat,
            {swapchainExtent.width, swapchainExtent.height, 1},
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment);

        depthImage = device.getLogicalDevice().createImage(imageCreateInfo);

        device.allocateAndBindImage(depthImageMemory, depthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);

        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(),
                                                    depthImage,
                                                    vk::ImageViewType::e2D,
                                                    depthFormat,
                                                    {},
                                                    {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});

        depthImageView = device.getLogicalDevice().createImageView(imageViewCreateInfo);
    }

    void Swapchain::createRenderPass()
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
                                                                vk::ImageLayout::eColorAttachmentOptimal);

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

    void Swapchain::createUIRenderPass()
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
            vk::ImageLayout::eColorAttachmentOptimal,
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
            vk::AccessFlagBits::eNone,
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

    void Swapchain::createFrameBuffers()
    {
        std::array<vk::ImageView, 2> renderAttachments;
        std::array<vk::ImageView, 1> uiAttachment;

        vk::FramebufferCreateInfo framebufferCreateInfo(
            vk::FramebufferCreateFlags(), 
            nullptr, 
            nullptr, 
            swapchainExtent.width, 
            swapchainExtent.height, 
            1);
        
        mainFrameBuffers.reserve(imageViews.size());
        uiFrameBuffers.reserve(imageViews.size());

        for (const auto& imageView : imageViews)
        {
            renderAttachments[0] = uiAttachment[0] = imageView;
            renderAttachments[1] = depthImageView;

            framebufferCreateInfo.setAttachments(renderAttachments);
            framebufferCreateInfo.setRenderPass(mainRenderPass);
            mainFrameBuffers.push_back(device.getLogicalDevice().createFramebuffer(framebufferCreateInfo));

            framebufferCreateInfo.setAttachments(uiAttachment);
            framebufferCreateInfo.setRenderPass(uiRenderPass);
            uiFrameBuffers.push_back(device.getLogicalDevice().createFramebuffer(framebufferCreateInfo));
        }
    }

    void Swapchain::createSyncObject()
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

    vk::Result Swapchain::acquireNextImage(uint32_t* imageIndex, uint32_t& currentFrame)
    {
        if (device.getLogicalDevice().waitForFences(inFlightFences[currentFrame], true, UINT64_MAX) != vk::Result::eSuccess)
            throw std::runtime_error("Something's wrong when waiting for fences");

        return device.getLogicalDevice().acquireNextImageKHR(
            swapchain, 
            std::numeric_limits<uint64_t>::max(), 
            imageAvailableSemaphore[currentFrame], 
            nullptr, 
            imageIndex);
    }

    void Swapchain::loadTexture(const std::string &filename)
    {
        std::ifstream file;
        vk::Format format = vk::Format::eR8G8B8A8Srgb;

        ktxResult ktxResult;
        ktxTexture* ktxTexture;

        file.open(filename);
        if(!file)
            throw std::runtime_error("Cannot open image file");
        file.close();

        ktxResult = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
        assert(ktxResult == KTX_SUCCESS);

        textureProperties.width = ktxTexture->baseWidth;
        textureProperties.height = ktxTexture->baseHeight;
        textureProperties.mipLevels = ktxTexture->numLevels;
        ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        device.createBuffer(ktxTexture->baseWidth * ktxTexture->baseHeight * 4, 
                            vk::BufferUsageFlagBits::eTransferSrc, 
                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
                            stagingBuffer, 
                            stagingBufferMemory);

        vk::MemoryRequirements memRequirements = device.getLogicalDevice().getBufferMemoryRequirements(stagingBuffer);
        
        void* data;
        data = device.getLogicalDevice().mapMemory(stagingBufferMemory, 0, memRequirements.size);
        memcpy(data, ktxTextureData, ktxTextureSize);
        device.getLogicalDevice().unmapMemory(stagingBufferMemory);

        std::vector<vk::BufferImageCopy> bufferCopyRegions;
        bufferCopyRegions.reserve(textureProperties.mipLevels);
        uint32_t offset = 0;

        for (uint32_t i = 0; i < textureProperties.mipLevels; ++i)
        {
            ktx_size_t offset;
            KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
            assert(ret == KTX_SUCCESS);

            vk::BufferImageCopy bufferCopyRegion;
            bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferCopyRegion.imageSubresource.mipLevel = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> i;
            bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> i;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;
            bufferCopyRegion.bufferRowLength = 0;
            bufferCopyRegion.bufferImageHeight = 0;
            bufferCopyRegion.imageOffset.x = 0;
            bufferCopyRegion.imageOffset.y = 0;
            bufferCopyRegion.imageOffset.z = 0;

            bufferCopyRegions.push_back(bufferCopyRegion);
        }

        vk::ImageCreateInfo imageCreateInfo(
            vk::ImageCreateFlags(),
            vk::ImageType::e2D,
            format,
            {textureProperties.width, textureProperties.height, 1},
            textureProperties.mipLevels,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 
            vk::SharingMode::eExclusive);

        textureProperties.image = device.getLogicalDevice().createImage(imageCreateInfo);

        device.allocateAndBindImage(textureProperties.deviceMemory, textureProperties.image, vk::MemoryPropertyFlagBits::eDeviceLocal);

        vk::CommandBuffer copyCmd;
        device.beginSingleTimeCommands(copyCmd);

        vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, textureProperties.mipLevels, 0, 1);

        vk::ImageMemoryBarrier imageMemoryBarrier(vk::AccessFlagBits::eNone,
                                                vk::AccessFlagBits::eTransferWrite,
                                                vk::ImageLayout::eUndefined,
                                                vk::ImageLayout::eTransferDstOptimal,
                                                0,
                                                0,
                                                textureProperties.image,
                                                subResourceRange);

        copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost,
                                vk::PipelineStageFlagBits::eTransfer,
                                vk::DependencyFlags(),
                                nullptr,
                                nullptr,
                                imageMemoryBarrier);

        copyCmd.copyBufferToImage(stagingBuffer, textureProperties.image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);

        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                vk::PipelineStageFlagBits::eFragmentShader, 
                                vk::DependencyFlags(), 
                                nullptr, 
                                nullptr, 
                                imageMemoryBarrier);

        textureProperties.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        device.endSingleTimeCommand(copyCmd);

        device.getLogicalDevice().freeMemory(stagingBufferMemory);
        device.getLogicalDevice().destroyBuffer(stagingBuffer);

        ktxTexture_Destroy(ktxTexture);

        vk::SamplerCreateInfo samplerCreateInfo(vk::SamplerCreateFlags(),
                                            vk::Filter::eLinear,
                                            vk::Filter::eLinear,
                                            vk::SamplerMipmapMode::eLinear,
                                            vk::SamplerAddressMode::eRepeat,
                                            vk::SamplerAddressMode::eRepeat,
                                            vk::SamplerAddressMode::eRepeat,
                                            0.f,
                                            true,
                                            device.getPhysicalDevice().getProperties().limits.maxSamplerAnisotropy,
                                            false,
                                            vk::CompareOp::eNever,
                                            0.f,
                                            textureProperties.mipLevels,
                                            vk::BorderColor::eFloatOpaqueWhite, 
                                            false);

        textureProperties.sampler = device.getLogicalDevice().createSampler(samplerCreateInfo);

        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), 
                                                    textureProperties.image, 
                                                    vk::ImageViewType::e2D, 
                                                    format, 
                                                    {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA},
                                                    {vk::ImageAspectFlagBits::eColor, 0, textureProperties.mipLevels, 0, 1});

        textureProperties.imageView = device.getLogicalDevice().createImageView(imageViewCreateInfo);
    }
}

