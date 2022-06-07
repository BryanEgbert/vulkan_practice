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

TriangleDevice::TriangleDevice(const char* appName, TriangleWindow& window) 
    : appName{appName}, window{window}
{
    createInstance();
    window.createSurface(static_cast<VkInstance>(instance), &surface);
    createDevice();
    createCommandPool();
}

TriangleDevice::~TriangleDevice()
{
    std::cout << "deleting command pool...\n";
    device.destroyCommandPool(mainCommandPool);

    std::cout << "deleting device...\n";
    device.destroy();

    std::cout << "deleting surface...\n";
    instance.destroySurfaceKHR(surface);
    
    std::cout << "destroying debug utils...\n";
    instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);

    std::cout << "deleting instance...\n";
    instance.destroy();
}

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


VKAPI_ATTR VkBool32 VKAPI_CALL TriangleDevice::debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

bool TriangleDevice::checkRequiredLayers(const std::vector<const char*>& instanceLayers)
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

uint32_t TriangleDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    memProperties = physicalDevice.getMemoryProperties();
    for (int i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    
    throw std::runtime_error("Failed to find suitable memory type");
}


void TriangleDevice::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer&  buffer, vk::DeviceMemory& bufferMemory)
{
    vk::BufferCreateInfo bufferCreateInfo(
        vk::BufferCreateFlags(),
        size,
        usage     
    );

    buffer = device.createBuffer(bufferCreateInfo);

    std::cout << "Memory Type Count: "<< memProperties.memoryTypeCount << '\n'
              << "Memory Heap Count: "<< memProperties.memoryHeapCount << '\n';

    for (int i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        std::cout << "Memory heap " << i << ": " << vk::to_string(memProperties.memoryHeaps[i].flags) << '\n'
                  << "Memory type " << i << ": " << vk::to_string(memProperties.memoryTypes[i].propertyFlags) << "\tHeap index: " << memProperties.memoryTypes[i].heapIndex << '\n';
    }

    memRequirements = device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        findMemoryType(memRequirements.memoryTypeBits, properties));

    bufferMemory = device.allocateMemory(allocInfo);

    device.bindBufferMemory(buffer, bufferMemory, 0);
}

void TriangleDevice::beginSingleTimeCommands(vk::CommandBuffer& cmdBuffer)
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

void TriangleDevice::endSingleTimeCommand(vk::CommandBuffer& cmdBuffer)
{
    cmdBuffer.end();

    vk::SubmitInfo submitInfo(
        nullptr, nullptr, cmdBuffer, nullptr
    );

    graphicsQueue.submit(submitInfo);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(mainCommandPool, cmdBuffer);
}

void TriangleDevice::copyBuffer(vk::Buffer& srcBuffer, vk::Buffer& dstBuffer, vk::DeviceSize size)
{
    std::array<vk::CommandBuffer, 1> commandBuffer;
    beginSingleTimeCommands(commandBuffer[0]);
   
    vk::BufferCopy copyRegion({}, {}, size);
    commandBuffer[0].copyBuffer(srcBuffer, dstBuffer, copyRegion);

    endSingleTimeCommand(commandBuffer[0]);
}

void TriangleDevice::allocateAndBindImage(vk::DeviceMemory &imageMemory, vk::Image &image, vk::MemoryPropertyFlags properties)
{
    memRequirements = device.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size, 
        findMemoryType(memRequirements.memoryTypeBits, properties));

    imageMemory = device.allocateMemory(allocInfo);

    device.bindImageMemory(image, imageMemory, 0);

}

void TriangleDevice::createInstance()
{
    if (!TriangleDevice::checkRequiredLayers(validationLayers))
    {
      std::cout << "Cannot find required layer(s)" << std::endl;
      exit( 1 );
    }

    std::vector<const char*> instanceExtensions(window.glfwExtensions, window.glfwExtensions + window.glfwExtensionCount);
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    vk::ApplicationInfo appInfo(appName, 1, "No Engine", 1, VK_API_VERSION_1_3);

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags( vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
                                                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo(
        {}, severityFlags, messageTypeFlags, &TriangleDevice::debugMessageFunc);

    vk::InstanceCreateInfo instanceCreateInfo(
        {}, &appInfo, validationLayers, instanceExtensions, (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerCreateInfo);
    instance = vk::createInstance(instanceCreateInfo);

    TriangleDevice::createDebugMessenger(debugMessengerCreateInfo);
}

void TriangleDevice::createDevice()
{
    physicalDevice = instance.enumeratePhysicalDevices().front();

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    for (auto queue : queueFamilyProperties)
    {
        std::cout << "Queue count: " << queue.queueCount << '\t'
                  << "timestamp valid bits: " << queue.timestampValidBits << '\t'
                  << vk::to_string(queue.queueFlags) << '\n';
    }

    auto propertyIterator = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(), 
        [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });

    queueFamilyIndex.graphics = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), propertyIterator));
    assert( queueFamilyIndex.graphics < queueFamilyProperties.size() );
    std::cout << "graphics queue family index: " << queueFamilyIndex.graphics << '\n';

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

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndex.graphics, 1, &queuePriority);
    device = physicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo, validationLayers, deviceExtensions, &deviceFeatures));

    graphicsQueue = device.getQueue(queueFamilyIndex.graphics, 0);
    presentQueue = device.getQueue(queueFamilyIndex.present, 0);
}

void TriangleDevice::createCommandPool()
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex.graphics);
    mainCommandPool = device.createCommandPool(commandPoolCreateInfo);
}

void TriangleDevice::createDebugMessenger(vk::DebugUtilsMessengerCreateInfoEXT& debugMessengerCreateInfo)
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