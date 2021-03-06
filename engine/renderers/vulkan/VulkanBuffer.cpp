/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <sstream>
#include <limits>
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanBuffer::VulkanBuffer ( const VulkanRenderer & aVulkanRenderer ) : mVulkanRenderer ( aVulkanRenderer )
    {
    }
    VulkanBuffer::VulkanBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void *aData ) :
        mVulkanRenderer ( aVulkanRenderer ), mSize ( aSize ), mUsage ( aUsage ), mProperties ( aProperties )
    {
        try
        {
            Initialize ( aData );
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        Finalize();
    }

    void VulkanBuffer::Initialize ( const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void * aData )
    {
        if ( mDeviceMemory != VK_NULL_HANDLE || mBuffer != VK_NULL_HANDLE )
        {
            throw ( std::runtime_error ( "Buffer already initialized." ) );
        }
        mSize = aSize;
        mUsage = aUsage;
        mProperties = aProperties;
        Initialize ( aData );
    }

    const VkBuffer& VulkanBuffer::GetBuffer() const
    {
        return mBuffer;
    }

    void VulkanBuffer::WriteMemory ( const VkDeviceSize aOffset, const VkDeviceSize aSize, const void * aData ) const
    {
        if ( aData )
        {
            if ( ( mProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) )
            {
                void* data = Map ( aOffset, aSize );
                memcpy ( data, aData, aSize );
                Unmap();
            }
            else if ( ( mProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) && ( mUsage & VK_BUFFER_USAGE_TRANSFER_DST_BIT ) )
            {
                VulkanBuffer source ( mVulkanRenderer, aSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, aData );
                VkCommandBufferAllocateInfo command_buffer_allocate_info{};
                command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                command_buffer_allocate_info.commandPool = mVulkanRenderer.GetCommandPool();
                command_buffer_allocate_info.commandBufferCount = 1;

                VkCommandBuffer command_buffer;
                vkAllocateCommandBuffers ( mVulkanRenderer.GetDevice(), &command_buffer_allocate_info, &command_buffer );

                VkCommandBufferBeginInfo begin_info = {};
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer ( command_buffer, &begin_info );

                VkBufferCopy copy_region = {};
                copy_region.size = aSize;
                copy_region.srcOffset  = copy_region.dstOffset = aOffset;
                vkCmdCopyBuffer ( command_buffer, source.GetBuffer(), mBuffer, 1, &copy_region );

                vkEndCommandBuffer ( command_buffer );

                VkSubmitInfo submit_info = {};
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &command_buffer;
                vkQueueSubmit ( mVulkanRenderer.GetQueue(), 1, &submit_info, VK_NULL_HANDLE );
                vkQueueWaitIdle ( mVulkanRenderer.GetQueue() );
                vkFreeCommandBuffers ( mVulkanRenderer.GetDevice(), mVulkanRenderer.GetCommandPool(), 1, &command_buffer );
            }
        }
    }

    void * VulkanBuffer::Map ( const VkDeviceSize aOffset, const VkDeviceSize aSize ) const
    {
        void* data = nullptr;
        if ( mProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
        {
            if ( VkResult result = vkMapMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory, aOffset, aSize, 0, &data ) )
            {
                std::ostringstream stream;
                stream << "vkMapMemory failed for buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        else
        {
            throw std::runtime_error ( "The VkBuffer VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT property must be set to be able to map buffer memory." );
        }
        return data;
    }

    void VulkanBuffer::Unmap() const
    {
        vkUnmapMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory );
    }

    VkDeviceSize VulkanBuffer::GetSize() const
    {
        return mSize;
    }

    void VulkanBuffer::Initialize ( const void* aData )
    {
        if ( !mSize )
        {
            return;
        }
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = nullptr;
        buffer_create_info.flags = 0;
        buffer_create_info.size = mSize;
        buffer_create_info.usage = mUsage;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 0;
        buffer_create_info.pQueueFamilyIndices = nullptr;

        if ( VkResult result = vkCreateBuffer ( mVulkanRenderer.GetDevice(), &buffer_create_info, nullptr, &mBuffer ) )
        {
            std::ostringstream stream;
            stream << "vkCreateBuffer failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements ( mVulkanRenderer.GetDevice(), mBuffer, &memory_requirements );

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = mVulkanRenderer.GetMemoryTypeIndex ( mProperties );

        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "No suitable memory type found for buffer." );
        }

        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &mDeviceMemory ) )
        {
            std::ostringstream stream;
            stream << "vkAllocateMemory failed for buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkBindBufferMemory ( mVulkanRenderer.GetDevice(), mBuffer, mDeviceMemory, 0 );

        WriteMemory ( 0, mSize, aData );
    }

    void VulkanBuffer::Finalize()
    {
        if ( mDeviceMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory, nullptr );
            mDeviceMemory = VK_NULL_HANDLE;
        }
        if ( mBuffer != VK_NULL_HANDLE )
        {
            vkDestroyBuffer ( mVulkanRenderer.GetDevice(), mBuffer, nullptr );
            mBuffer = VK_NULL_HANDLE;
        }
    }
}
