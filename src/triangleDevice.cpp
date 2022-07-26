#include "triangleDevice.hpp"
#include "triangleWindow.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <sys/types.h>
#include <vector>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(  VkInstance instance,
                                                                const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkDebugUtilsMessengerEXT* pMessenger)
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}



namespace triangle
{
    Device::Device(const char* appName, Window& window) 
        : appName{appName}, window{window}
    {
        createInstance();
        window.createSurface(static_cast<VkInstance>(instance), &surface);
        createDevice();
        createCommandPool();
    }

    Device::~Device()
    {
        device.destroyCommandPool(mainCommandPool);
        device.destroy();
        instance.destroySurfaceKHR(surface);    
        if (enableValidationLayers)
            instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
        instance.destroy();
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Device::debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        std::ostringstream message;

        message << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": "
                << vk::to_string(static_cast<vk::DebugUtilsMessengerCreateFlagsEXT>(messageTypes)) << "\t"
                << "message: " << pCallbackData->pMessage << '\n';

        std::cout << message.str() << '\n';

        return false;
    }

    bool Device::checkRequiredLayers(const std::vector<const char*>& instanceLayers)
    {
        bool layerFound = false;
        std::vector<vk::LayerProperties> props = vk::enumerateInstanceLayerProperties();

        for (const auto& requiredLayer : instanceLayers)
        {
            for (const auto& layer : props)
            {
                if (strcmp(layer.layerName, requiredLayer) == 0)
                {
                    layerFound = true;
                    break;
                }

            }
        }

        return layerFound;
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        memProperties = physicalDevice.getMemoryProperties();
        for (int i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }
        
        throw std::runtime_error("Failed to find suitable memory type");
    }


    void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer&  buffer, vk::DeviceMemory& bufferMemory)
    {
        vk::BufferCreateInfo bufferCreateInfo(
            vk::BufferCreateFlags(),
            size,
            usage     
        );

        buffer = device.createBuffer(bufferCreateInfo);

        memRequirements = device.getBufferMemoryRequirements(buffer);

        vk::MemoryAllocateInfo allocInfo(
            memRequirements.size,
            findMemoryType(memRequirements.memoryTypeBits, properties));

        bufferMemory = device.allocateMemory(allocInfo);

        device.bindBufferMemory(buffer, bufferMemory, 0);
    }

    void Device::beginSingleTimeCommands(vk::CommandBuffer& cmdBuffer)
    {
        vk::CommandBufferAllocateInfo allocInfo(
            mainCommandPool,
            vk::CommandBufferLevel::ePrimary,
            1
        );

        cmdBuffer = device.allocateCommandBuffers(allocInfo).front();

        vk::CommandBufferBeginInfo beginInfo(
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        );

        cmdBuffer.begin(beginInfo);
    }

    void Device::endSingleTimeCommand(vk::CommandBuffer& cmdBuffer)
    {
        cmdBuffer.end();

        vk::SubmitInfo submitInfo(
            nullptr, nullptr, cmdBuffer, nullptr
        );

        graphicsQueue.submit(submitInfo);
        graphicsQueue.waitIdle();

        device.freeCommandBuffers(mainCommandPool, cmdBuffer);
    }

    void Device::copyBuffer(vk::Buffer& srcBuffer, vk::Buffer& dstBuffer, vk::DeviceSize size)
    {
        std::array<vk::CommandBuffer, 1> commandBuffer;
        beginSingleTimeCommands(commandBuffer[0]);
    
        vk::BufferCopy copyRegion({}, {}, size);
        commandBuffer[0].copyBuffer(srcBuffer, dstBuffer, copyRegion);

        endSingleTimeCommand(commandBuffer[0]);
    }

    void Device::allocateAndBindImage(vk::DeviceMemory &imageMemory, vk::Image &image, vk::MemoryPropertyFlags properties)
    {
        memRequirements = device.getImageMemoryRequirements(image);

        vk::MemoryAllocateInfo allocInfo(
            memRequirements.size, 
            findMemoryType(memRequirements.memoryTypeBits, properties));

        imageMemory = device.allocateMemory(allocInfo);

        device.bindImageMemory(image, imageMemory, 0);

    }

    void Device::createInstance()
    {
        if (!Device::checkRequiredLayers(validationLayers))
        {
            std::cout << "Cannot find required layer(s)" << std::endl;
            exit( 1 );
        }

        std::vector<const char*> instanceExtensions(window.glfwExtensions, window.glfwExtensions + window.glfwExtensionCount);

        vk::ApplicationInfo appInfo(appName, 1, "No Engine", 1, VK_API_VERSION_1_3);

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags( vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
                                                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);


        vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo, {}, instanceExtensions);

        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo(
            {}, severityFlags, messageTypeFlags, &Device::debugMessageFunc);

        if (enableValidationLayers)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            instanceCreateInfo.setPEnabledLayerNames(validationLayers);
            instanceCreateInfo.setPEnabledExtensionNames(instanceExtensions);
            instanceCreateInfo.setPNext(reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugMessengerCreateInfo));
        }

        instance = vk::createInstance(instanceCreateInfo);

        if (enableValidationLayers)
            Device::createDebugMessenger(debugMessengerCreateInfo);
    }

    void Device::createDevice()
    {
        physicalDevice = instance.enumeratePhysicalDevices().front();

        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        auto propertyIterator = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(), 
            [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });

        queueFamilyIndex.graphics = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), propertyIterator));
        assert( queueFamilyIndex.graphics < queueFamilyProperties.size() );

        queueFamilyIndex.present = physicalDevice.getSurfaceSupportKHR( static_cast<uint32_t>( queueFamilyIndex.graphics ), surface )
                                    ? queueFamilyIndex.graphics
                                    : queueFamilyProperties.size();
        if ( queueFamilyIndex.present == queueFamilyProperties.size() )
        {
            for ( size_t i = 0; i < queueFamilyProperties.size(); i++ )
            {
                if ( ( queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics ) &&
                        physicalDevice.getSurfaceSupportKHR( static_cast<uint32_t>( i ), surface ) )
                {
                    queueFamilyIndex.graphics = static_cast<uint32_t>( i );
                    queueFamilyIndex.present  = i;
                    break;
                }
            }
            if ( queueFamilyIndex.present == queueFamilyProperties.size() )
            {
                for ( size_t i = 0; i < queueFamilyProperties.size(); i++ )
                {
                    if ( physicalDevice.getSurfaceSupportKHR( static_cast<uint32_t>( i ), surface ) )
                    {
                        queueFamilyIndex.present = i;
                        break;
                    }
                }
            }
        }
        if ( ( queueFamilyIndex.graphics == queueFamilyProperties.size() ) || ( queueFamilyIndex.present == queueFamilyProperties.size() ) )
        {
            throw std::runtime_error( "Could not find a queue for graphics or present -> terminating" );
        }

        float queuePriority = 1.0f;
        vk::PhysicalDeviceFeatures deviceFeatures;
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndex.graphics, 1, &queuePriority);

        vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo, {}, deviceExtensions, &deviceFeatures);
        if (enableValidationLayers)
        {
            deviceCreateInfo.setPEnabledLayerNames(validationLayers);
        }
        device = physicalDevice.createDevice(deviceCreateInfo);

        graphicsQueue = device.getQueue(queueFamilyIndex.graphics, 0);
        presentQueue = device.getQueue(queueFamilyIndex.present, 0);
    }

    void Device::createCommandPool()
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex.graphics);
        mainCommandPool = device.createCommandPool(commandPoolCreateInfo);
    }

    void Device::createDebugMessenger(vk::DebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo)
    {
        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

        if (!pfnVkCreateDebugUtilsMessengerEXT || !pfnVkDestroyDebugUtilsMessengerEXT)
        {
            std::cout << "unable to find debug utils ext\n";
            exit(1);
        }

        debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo);
    }
}