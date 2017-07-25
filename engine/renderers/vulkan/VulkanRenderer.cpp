/*
Copyright (C) 2016,2017 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*  Reference and Credits:
    Vulkan SDK Demo code.
    https://www.youtube.com/playlist?list=PLUXvZMiAqNbK8jd7s52BIDtCbZnKNGp0P
    https://vulkan.lunarg.com/app/docs/v1.0.8.0/layers
    http://gpuopen.com/using-the-vulkan-validation-layers/?utm_source=silverpop&utm_medium=email&utm_campaign=25324445&utm_term=link-article2&utm_content=p-global-developer-hcnewsflash-april-2016%20%281%29:&spMailingID=25324445&spUserID=NzI5Mzc5ODY4NjQS1&spJobID=783815030&spReportId=NzgzODE1MDMwS0
*/

#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>
#include <algorithm>
#include "aeongames/LogLevel.h"
#include "aeongames/Memory.h"
#include "aeongames/Model.h"
#include "VulkanRenderer.h"
#include "VulkanWindow.h"
#include "VulkanUtilities.h"
#include "VulkanPipeline.h"
#include "VulkanMesh.h"
#include "VulkanModel.h"
#include "math/3DMath.h"

namespace AeonGames
{
    VulkanRenderer::VulkanRenderer ( bool aValidate ) : mValidate ( aValidate )
    {
        try
        {
            SetupLayersAndExtensions();
            if ( mValidate )
            {
                SetupDebug();
            }
            InitializeInstance();
            if ( mValidate )
            {
                /* LoadFunctions currently only loads
                    Debug Functions. */
                LoadFunctions();
                InitializeDebug();
            }
            InitializeDevice();
            InitializeSemaphores();
            InitializeFence();
            InitializeRenderPass();
            InitializeMatricesUniform();
            InitializeCommandPool();
        }
        catch ( ... )
        {
            FinalizeCommandPool();
            FinalizeMatricesUniform();
            FinalizeRenderPass();
            FinalizeFence();
            FinalizeSemaphores();
            FinalizeDevice();
            FinalizeDebug();
            FinalizeInstance();
            throw;
        }
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkQueueWaitIdle ( mVkQueue );
        for ( auto& i : mModelMap )
        {
            /** @note This is here
            because we need any allocated
            models to be destroyed before
            the device is.*/
            i.second.reset();
        }
        FinalizeCommandPool();
        FinalizeMatricesUniform();
        FinalizeRenderPass();
        FinalizeFence();
        FinalizeSemaphores();
        FinalizeDevice();
        FinalizeDebug();
        FinalizeInstance();
    }

    const VkDevice & VulkanRenderer::GetDevice() const
    {
        return mVkDevice;
    }

    const VkQueue & VulkanRenderer::GetQueue() const
    {
        return mVkQueue;
    }

    const VkFence & VulkanRenderer::GetFence() const
    {
        return mVkFence;
    }

    const VkInstance & VulkanRenderer::GetInstance() const
    {
        return mVkInstance;
    }

    const VkPhysicalDevice & VulkanRenderer::GetPhysicalDevice() const
    {
        return mVkPhysicalDevice;
    }

    const VkPhysicalDeviceMemoryProperties & VulkanRenderer::GetPhysicalDeviceMemoryProperties() const
    {
        return mVkPhysicalDeviceMemoryProperties;
    }

    const VkRenderPass & VulkanRenderer::GetRenderPass() const
    {
        return mVkRenderPass;
    }

    const VkFormat & VulkanRenderer::GetDepthStencilFormat() const
    {
        return mVkDepthStencilFormat;
    }

    const VkSurfaceFormatKHR & VulkanRenderer::GetSurfaceFormatKHR() const
    {
        return mVkSurfaceFormatKHR;
    }

    const VkCommandBuffer & VulkanRenderer::GetCommandBuffer() const
    {
        return mVkCommandBuffer;
    }

    uint32_t VulkanRenderer::GetQueueFamilyIndex() const
    {
        return mQueueFamilyIndex;
    }

    uint32_t VulkanRenderer::GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const
    {
        for ( uint32_t i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
        {
            if ( ( mVkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
                   ( aVkMemoryPropertyFlags ) ) == ( aVkMemoryPropertyFlags ) )
            {
                return i;
            }
        }
        return std::numeric_limits<uint32_t>::max();
    }

    uint32_t VulkanRenderer::FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const
    {
        for ( uint32_t i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
        {
            if ( ( typeFilter & ( 1 << i ) ) && ( mVkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties ) == properties )
            {
                return i;
            }
        }
        return std::numeric_limits<uint32_t>::max();
    }

    const VkBuffer & VulkanRenderer::GetMatricesUniformBuffer() const
    {
        return mMatricesUniformBuffer;
    }

    void VulkanRenderer::LoadFunctions()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        if ( !mFunctionsLoaded && mVkInstance )
        {
            if ( ( vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkCreateDebugReportCallbackEXT" ) ) ) == nullptr )
            {
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkCreateDebugReportCallbackEXT" );
            }
            if ( ( vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkDestroyDebugReportCallbackEXT" ) ) ) == nullptr )
            {
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkDestroyDebugReportCallbackEXT" );
            }
            mFunctionsLoaded = true;
        }
    }


    void VulkanRenderer::SetupDebug()
    {
        mDebugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        mDebugReportCallbackCreateInfo.pfnCallback = DebugCallback;
        mDebugReportCallbackCreateInfo.flags =
            VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_DEBUG_BIT_EXT;

        //mInstanceLayerNames.emplace_back ( "VK_LAYER_LUNARG_standard_validation" );

        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

        //mDeviceLayerNames.emplace_back ( "VK_LAYER_LUNARG_standard_validation" );
    }

    void VulkanRenderer::InitializeDebug()
    {
        if ( VkResult result = vkCreateDebugReportCallbackEXT ( mVkInstance, &mDebugReportCallbackCreateInfo, nullptr, &mVkDebugReportCallbackEXT ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer debug report callback. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeMatricesUniform()
    {
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = nullptr;
        buffer_create_info.flags = 0;
        buffer_create_info.size = sizeof ( Matrices );
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 0;
        buffer_create_info.pQueueFamilyIndices = nullptr;

        if ( VkResult result = vkCreateBuffer ( mVkDevice, &buffer_create_info, nullptr, &mMatricesUniformBuffer ) )
        {
            std::ostringstream stream;
            stream << "vkCreateBuffer failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements ( mVkDevice, mMatricesUniformBuffer, &memory_requirements );

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = GetMemoryTypeIndex ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "No suitable memory type found for mesh buffers" );
        }

        if ( VkResult result = vkAllocateMemory ( mVkDevice, &memory_allocate_info, nullptr, &mMatricesUniformMemory ) )
        {
            std::ostringstream stream;
            stream << "vkAllocateMemory failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkBindBufferMemory ( mVkDevice, mMatricesUniformBuffer, mMatricesUniformMemory, 0 );
    }

    void VulkanRenderer::SetupLayersAndExtensions()
    {
        mInstanceExtensionNames.push_back ( VK_KHR_SURFACE_EXTENSION_NAME );
#ifdef VK_USE_PLATFORM_WIN32_KHR
        mInstanceExtensionNames.push_back ( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#elif defined (VK_USE_PLATFORM_XLIB_KHR)
        mInstanceExtensionNames.push_back ( VK_KHR_XLIB_SURFACE_EXTENSION_NAME );
#endif
        mDeviceExtensionNames.push_back ( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    }

    void VulkanRenderer::InitializeInstance()
    {
        VkInstanceCreateInfo instance_create_info {};
        VkApplicationInfo application_info {};

        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.apiVersion = 0;
        application_info.applicationVersion = VK_MAKE_VERSION ( 0, 1, 0 );
        application_info.pApplicationName = "AeonEngine Vulkan Renderer";

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledLayerCount = static_cast<uint32_t> ( mInstanceLayerNames.size() );
        instance_create_info.ppEnabledLayerNames = mInstanceLayerNames.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t> ( mInstanceExtensionNames.size() );
        instance_create_info.ppEnabledExtensionNames = mInstanceExtensionNames.data();
        if ( mValidate )
        {
            instance_create_info.pNext = &mDebugReportCallbackCreateInfo;
        }
        if ( VkResult result = vkCreateInstance ( &instance_create_info, nullptr, &mVkInstance ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer instance. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeDevice()
    {
        if ( !mVkInstance )
        {
            throw std::runtime_error ( "mVkInstance is a nullptr." );
        }
        {
            uint32_t physical_device_count;
            vkEnumeratePhysicalDevices ( mVkInstance, &physical_device_count, nullptr );

            if ( physical_device_count == 0 )
            {
                throw std::runtime_error ( "No VulkanRenderer physical device found" );
            }

            std::vector<VkPhysicalDevice> physical_device_list ( physical_device_count );
            vkEnumeratePhysicalDevices ( mVkInstance, &physical_device_count, physical_device_list.data() );

            mVkPhysicalDevice = physical_device_list[0];
            vkGetPhysicalDeviceProperties ( mVkPhysicalDevice, &mVkPhysicalDeviceProperties );
            vkGetPhysicalDeviceMemoryProperties ( mVkPhysicalDevice, &mVkPhysicalDeviceMemoryProperties );
        }

        VkDeviceCreateInfo device_create_info{};
        VkDeviceQueueCreateInfo device_queue_create_info{};
        {
            uint32_t family_properties_count;
            vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &family_properties_count, nullptr );
            if ( family_properties_count == 0 )
            {
                throw std::runtime_error ( "VulkanRenderer physical device has no queue family properties." );
            }
            std::vector<VkQueueFamilyProperties> family_properties_list ( family_properties_count );
            vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &family_properties_count, family_properties_list.data() );
            bool graphics_queue_family_found = false;
            for ( auto family_property = family_properties_list.begin();
                  family_property != family_properties_list.end();
                  ++family_property )
            {
                if ( family_property->queueFlags & VK_QUEUE_GRAPHICS_BIT )
                {
                    graphics_queue_family_found = true;
                    mQueueFamilyIndex =
                        static_cast<uint32_t> ( family_properties_list.begin() - family_property );
                    break;
                }
            }
            if ( !graphics_queue_family_found )
            {
                throw std::runtime_error ( "No graphics queue family found." );
            }
        }

        {
            uint32_t extension_property_count;
            vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr, &extension_property_count, nullptr );

            std::vector<VkExtensionProperties> extension_properties ( extension_property_count );
            vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr, &extension_property_count, extension_properties.data() );
            std::cout << "Vulkan Extensions" << std::endl;
            for ( auto& i : extension_properties )
            {
                std::cout << " " << i.extensionName << "t|\t" << i.specVersion << std::endl;
                if ( !strcmp ( i.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME ) )
                {
                    mDeviceExtensionNames.emplace_back ( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );
                }
            }
        }

        {
            uint32_t instance_layer_count;
            vkEnumerateInstanceLayerProperties ( &instance_layer_count, nullptr );
            std::vector<VkLayerProperties> instance_layer_list ( instance_layer_count );
            vkEnumerateInstanceLayerProperties ( &instance_layer_count, instance_layer_list.data() );
            std::cout << "VulkanRenderer Instance Layers" << std::endl;
            for ( auto& i : instance_layer_list )
            {
                std::cout << " " << i.layerName << "\t|\t" << i.description << std::endl;
            }
        }

        {
            uint32_t device_layer_count;
            vkEnumerateDeviceLayerProperties ( mVkPhysicalDevice, &device_layer_count, nullptr );
            std::vector<VkLayerProperties> device_layer_list ( device_layer_count );
            vkEnumerateInstanceLayerProperties ( &device_layer_count, device_layer_list.data() );
            std::cout << "VulkanRenderer Device Layers" << std::endl;
            for ( auto& i : device_layer_list )
            {
                std::cout << " " << i.layerName << "\t|\t" << i.description << std::endl;
            }
        }

        /// @todo Remove hardcoded Queue Info
        float queue_priorities[] {1.0f};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.queueFamilyIndex = mQueueFamilyIndex;
        device_queue_create_info.pQueuePriorities = queue_priorities;

        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &device_queue_create_info;
        device_create_info.enabledLayerCount = static_cast<uint32_t> ( mDeviceLayerNames.size() );
        device_create_info.ppEnabledLayerNames = mDeviceLayerNames.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t> ( mDeviceExtensionNames.size() );
        device_create_info.ppEnabledExtensionNames = mDeviceExtensionNames.data();

        /// @todo Grab best device rather than first one
        if ( VkResult result = vkCreateDevice ( mVkPhysicalDevice, &device_create_info, nullptr, &mVkDevice ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer device. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkGetDeviceQueue ( mVkDevice, mQueueFamilyIndex, 0, &mVkQueue );
    }

    void VulkanRenderer::InitializeSemaphores()
    {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if ( VkResult result = vkCreateSemaphore ( mVkDevice, &semaphore_create_info, nullptr, &mVkSignalSemaphore ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer semaphore. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeFence()
    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence ( mVkDevice, &fence_create_info, nullptr, &mVkFence );
    }

    void VulkanRenderer::InitializeRenderPass()
    {
        mVkSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
        mVkSurfaceFormatKHR.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        std::array<VkFormat, 5> try_formats
        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM
        };
        for ( auto format : try_formats )
        {
            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties ( mVkPhysicalDevice, format, &format_properties );
            if ( format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
            {
                mVkDepthStencilFormat = format;
                break;
            }
        }

        if ( std::find ( try_formats.begin(), try_formats.end(), mVkDepthStencilFormat ) == try_formats.end() )
        {
            std::ostringstream stream;
            stream << "Unable to find a suitable depth stencil format";
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::array<VkAttachmentDescription, 2> attachment_descriptions{ {} };
        attachment_descriptions[0].flags = 0;
        attachment_descriptions[0].format = mVkSurfaceFormatKHR.format;
        attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachment_descriptions[1].flags = 0;
        attachment_descriptions[1].format = mVkDepthStencilFormat;
        attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkAttachmentReference, 1> color_attachment_references{};
        color_attachment_references[0].attachment = 0;
        color_attachment_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_stencil_attachment_reference{};
        depth_stencil_attachment_reference.attachment = 1;
        depth_stencil_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpass_descriptions{};
        subpass_descriptions[0].flags = 0;
        subpass_descriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_descriptions[0].inputAttachmentCount = 0;
        subpass_descriptions[0].pInputAttachments = nullptr;
        subpass_descriptions[0].colorAttachmentCount = static_cast<uint32_t> ( color_attachment_references.size() );
        subpass_descriptions[0].pColorAttachments = color_attachment_references.data();
        subpass_descriptions[0].pResolveAttachments = nullptr;
        subpass_descriptions[0].pDepthStencilAttachment = &depth_stencil_attachment_reference;
        subpass_descriptions[0].preserveAttachmentCount = 0;
        subpass_descriptions[0].pPreserveAttachments = nullptr;

        VkSubpassDependency subpass_dependency = {};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = static_cast<uint32_t> ( attachment_descriptions.size() );
        render_pass_create_info.pAttachments = attachment_descriptions.data();
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &subpass_dependency;
        render_pass_create_info.subpassCount = static_cast<uint32_t> ( subpass_descriptions.size() );
        render_pass_create_info.pSubpasses = subpass_descriptions.data();
        vkCreateRenderPass ( mVkDevice, &render_pass_create_info, nullptr, &mVkRenderPass );
    }

    void VulkanRenderer::InitializeCommandPool()
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = mQueueFamilyIndex;
        vkCreateCommandPool ( mVkDevice, &command_pool_create_info, nullptr, &mVkCommandPool );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVkDevice, &command_buffer_allocate_info, &mVkCommandBuffer );
    }

    void VulkanRenderer::FinalizeCommandPool()
    {
        if ( mVkCommandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( mVkDevice, mVkCommandPool, nullptr );
            mVkCommandPool = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeDebug()
    {
        if ( mVkInstance && ( mVkDebugReportCallbackEXT != VK_NULL_HANDLE ) )
        {
            vkDestroyDebugReportCallbackEXT ( mVkInstance, mVkDebugReportCallbackEXT, nullptr );
            mVkDebugReportCallbackEXT = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeMatricesUniform()
    {
        if ( mMatricesUniformMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVkDevice, mMatricesUniformMemory, nullptr );
            mMatricesUniformMemory = VK_NULL_HANDLE;
        }
        if ( mMatricesUniformBuffer != VK_NULL_HANDLE )
        {
            vkDestroyBuffer ( mVkDevice, mMatricesUniformBuffer, nullptr );
            mMatricesUniformBuffer = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeInstance()
    {
        if ( mVkInstance != VK_NULL_HANDLE )
        {
            vkDestroyInstance ( mVkInstance, nullptr );
            mVkInstance = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeDevice()
    {
        if ( mVkDevice != VK_NULL_HANDLE )
        {
            vkDestroyDevice ( mVkDevice, nullptr );
            mVkDevice = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeSemaphores()
    {
        if ( mVkSignalSemaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore ( mVkDevice, mVkSignalSemaphore, nullptr );
            mVkSignalSemaphore = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeFence()
    {
        if ( mVkFence != VK_NULL_HANDLE )
        {
            vkDestroyFence ( mVkDevice, mVkFence, nullptr );
            mVkFence = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeRenderPass()
    {
        if ( mVkRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( mVkDevice, mVkRenderPass, nullptr );
        }
    }

    void VulkanRenderer::BeginRender ( void* aWindowId ) const
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [this, &aWindowId] ( const std::unique_ptr<VulkanWindow>& window ) -> bool
        {
            if ( window->GetWindowId() == aWindowId )
            {
                return true;
            }
            return false;
        } );
        if ( i != mWindowRegistry.end() )
        {
            ( *i )->AcquireNextImage();
            if ( VkResult result = vkWaitForFences ( mVkDevice, 1,
                                   &mVkFence,
                                   VK_TRUE, UINT64_MAX ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }
            if ( VkResult result = vkResetFences ( mVkDevice, 1,
                                                   &mVkFence ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }
            if ( VkResult result = vkQueueWaitIdle ( mVkQueue ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }

            VkCommandBufferBeginInfo command_buffer_begin_info{};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            if ( VkResult result = vkBeginCommandBuffer ( mVkCommandBuffer, &command_buffer_begin_info ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }

            VkRect2D render_area{ { 0, 0 }, { ( *i )->GetWidth(), ( *i )->GetHeight() } };
            /* [1] is depth/stencil [0] is color.*/
            std::array<VkClearValue, 2> clear_values{ { { 0 }, { 0 } } };
            clear_values[0].color.float32[0] = 0.5f;
            clear_values[0].color.float32[1] = 0.5f;
            clear_values[0].color.float32[2] = 0.5f;
            clear_values[0].color.float32[3] = 0.0f;
            clear_values[1].depthStencil.depth = 1.0f;
            clear_values[1].depthStencil.stencil = 0;

            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass = mVkRenderPass;
            render_pass_begin_info.framebuffer = ( *i )->GetActiveFrameBuffer();
            render_pass_begin_info.renderArea = render_area;
            render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
            render_pass_begin_info.pClearValues = clear_values.data();
            vkCmdBeginRenderPass ( mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
        }
    }

    void VulkanRenderer::EndRender ( void* aWindowId ) const
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [this, &aWindowId] ( const std::unique_ptr<VulkanWindow>& window ) -> bool
        {
            if ( window->GetWindowId() == aWindowId )
            {
                return true;
            }
            return false;
        } );
        if ( i != mWindowRegistry.end() )
        {
            vkCmdEndRenderPass ( mVkCommandBuffer );
            if ( VkResult result = vkEndCommandBuffer ( mVkCommandBuffer ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = nullptr;
            submit_info.pWaitDstStageMask = nullptr;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &mVkCommandBuffer;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &mVkSignalSemaphore;
            if ( VkResult result = vkQueueSubmit ( mVkQueue, 1, &submit_info, VK_NULL_HANDLE ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }
            std::array<VkResult, 1> result_array{ { VkResult::VK_SUCCESS } };
            VkPresentInfoKHR present_info{};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &mVkSignalSemaphore;
            present_info.swapchainCount = 1;
            present_info.pSwapchains = & ( *i )->GetSwapchain();
            present_info.pImageIndices = & ( *i )->GetActiveImageIndex();
            present_info.pResults = result_array.data();
            //present_info.pResults = nullptr;
            if ( VkResult result = vkQueuePresentKHR ( mVkQueue, &present_info ) )
            {
                std::cout << GetVulkanResultString ( result ) << std::endl;
            }
        }
    }

    void VulkanRenderer::Render ( void* aWindowId, const std::shared_ptr<Model> aModel ) const
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [this, &aWindowId] ( const std::unique_ptr<VulkanWindow>& window ) -> bool
        {
            if ( window->GetWindowId() == aWindowId )
            {
                return true;
            }
            return false;
        } );
        if ( i != mWindowRegistry.end() )
        {
            void* data = nullptr;
            if ( VkResult result = vkMapMemory ( mVkDevice, mMatricesUniformMemory, 0, VK_WHOLE_SIZE, 0, &data ) )
            {
                std::cout << "vkMapMemory failed for uniform buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            }
            memcpy ( data, &mMatrices, sizeof ( mMatrices ) );
            vkUnmapMemory ( mVkDevice, mMatricesUniformMemory );
            auto& model = mModelMap.at ( aModel );
            model->Render ( *i->get() );
        }
    }

    VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands() const
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        VkCommandBuffer command_buffer{};
        VkCommandBufferBeginInfo beginInfo{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVkDevice, &command_buffer_allocate_info, &command_buffer );
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer ( command_buffer, &beginInfo );
        return command_buffer;
    }

    void VulkanRenderer::EndSingleTimeCommands ( VkCommandBuffer aVkCommandBuffer ) const
    {
        vkEndCommandBuffer ( aVkCommandBuffer );
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &aVkCommandBuffer;
        vkQueueSubmit ( mVkQueue, 1, &submit_info, VK_NULL_HANDLE );
        vkQueueWaitIdle ( mVkQueue );
        vkFreeCommandBuffers ( mVkDevice, mVkCommandPool, 1, &aVkCommandBuffer );
    }

    bool VulkanRenderer::AllocateModelRenderData ( std::shared_ptr<Model> aModel )
    {
        if ( mModelMap.find ( aModel ) == mModelMap.end() )
        {
            /* We dont really need to cache Vulkan Models,
            since mModelMap IS our model cache.
            We DO need a deallocation function.*/
            mModelMap[aModel] = std::make_unique<VulkanModel> ( aModel, this );
        }
        return true;
    }

    bool VulkanRenderer::AddRenderingWindow ( void* aWindowId )
    {
        try
        {
            mWindowRegistry.emplace_back ( std::make_unique<VulkanWindow> ( aWindowId, this ) );
            return true;
        }
        catch ( std::runtime_error& e )
        {
            std::cout << e.what();
            return false;
        }
    }

    void VulkanRenderer::RemoveRenderingWindow ( void* aWindowId )
    {
        vkQueueWaitIdle ( mVkQueue );
        mWindowRegistry.erase (
            std::remove_if (
                mWindowRegistry.begin(),
                mWindowRegistry.end(),
                [this, &aWindowId] ( const std::unique_ptr<VulkanWindow>& window ) -> bool
        {
            if ( window->GetWindowId() == aWindowId )
            {
                return true;
            }
            return false;
        } ), mWindowRegistry.end() );
    }

    void VulkanRenderer::Resize ( void* aWindowId, uint32_t aWidth, uint32_t aHeight )
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [&aWindowId] ( const std::unique_ptr<VulkanWindow>& aWindow ) -> bool
        {
            return aWindow->GetWindowId() == aWindowId;
        } );
        if ( i != mWindowRegistry.end() )
        {
            if ( aWidth > 0 && aHeight > 0 )
            {
                ( *i )->Resize ( aWidth, aHeight );
            }
        }
    }

    void VulkanRenderer::UpdateMatrices()
    {
        /** @todo Either publish this function or
        add arguments so just some matrices are
        updated based on which one changed.*/
        // Update mViewProjectionMatrix
        Multiply4x4Matrix (
            mMatrices.mProjectionMatrix,
            mMatrices.mViewMatrix,
            mMatrices.mViewProjectionMatrix );
        // Update mModelViewMatrix
        Multiply4x4Matrix (
            mMatrices.mViewMatrix,
            mMatrices.mModelMatrix,
            mMatrices.mModelViewMatrix );
        // Update mModelViewProjectionMatrix
        Multiply4x4Matrix (
            mMatrices.mViewProjectionMatrix,
            mMatrices.mModelMatrix,
            mMatrices.mModelViewProjectionMatrix );
        /*  Calculate Normal Matrix
        Inverting a 3x3 matrix is cheaper than inverting a 4x4 matrix,
        so even if the shader alignment requires us to pad the 3x3 matrix into
        a 4x3 matrix we do these operations on a 3x3 basis.*/
        Extract3x3Matrix (
            mMatrices.mModelViewMatrix,
            mMatrices.mNormalMatrix );
        Invert3x3Matrix (
            mMatrices.mNormalMatrix,
            mMatrices.mNormalMatrix );
        Transpose3x3Matrix (
            mMatrices.mNormalMatrix,
            mMatrices.mNormalMatrix );
        Convert3x3To4x3 (
            mMatrices.mNormalMatrix,
            mMatrices.mNormalMatrix );
    }

    void VulkanRenderer::SetViewMatrix ( const float aMatrix[16] )
    {
        memcpy ( mMatrices.mViewMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void VulkanRenderer::SetProjectionMatrix ( const float aMatrix[16] )
    {
        memcpy ( mMatrices.mProjectionMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void VulkanRenderer::SetModelMatrix ( const float aMatrix[16] )
    {
        memcpy ( mMatrices.mModelMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }
}
