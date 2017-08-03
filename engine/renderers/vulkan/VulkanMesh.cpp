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

#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const std::shared_ptr<Mesh> aMesh, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer ) :
        mMesh ( aMesh ), mVulkanRenderer ( aVulkanRenderer )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }
    VulkanMesh::~VulkanMesh()
    {
        Finalize();
    }

    void VulkanMesh::Render() const
    {
        auto& triangle_groups = mMesh->GetTriangleGroups();
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            const VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers ( mVulkanRenderer->GetCommandBuffer(), 0, 1, &mBuffers[i].mVertexBuffer, &offset );
            if ( triangle_groups[i].mIndexCount )
            {
                vkCmdBindIndexBuffer ( mVulkanRenderer->GetCommandBuffer(),
                                       mBuffers[i].mIndexBuffer, 0,
                                       GetIndexType ( static_cast<AeonGames::Mesh::IndexType> ( triangle_groups[i].mIndexType ) ) );
                vkCmdDrawIndexed ( mVulkanRenderer->GetCommandBuffer(), triangle_groups[i].mIndexCount, 1, 0, 0, 1 );
            }
            else
            {
                vkCmdDraw ( mVulkanRenderer->GetCommandBuffer(), triangle_groups[i].mVertexCount, 1, 0, 0 );
            }
        }
    }

    void VulkanMesh::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        auto& triangle_groups = mMesh->GetTriangleGroups();
        auto& physical_device_memory_properties = mVulkanRenderer->GetPhysicalDeviceMemoryProperties();
        uint32_t memory_index = UINT32_MAX;
        for ( uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; ++i )
        {
            if ( ( physical_device_memory_properties.memoryTypes[i].propertyFlags &
                   ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) )
                 == ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) )
            {
                memory_index = i;
                break;
            }
        }
        if ( memory_index == UINT32_MAX )
        {
            throw std::runtime_error ( "No suitable memory type found for mesh buffers" );
        }
        mBuffers.resize ( triangle_groups.size() );
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            if ( triangle_groups[i].mVertexCount )
            {
                VkBufferCreateInfo buffer_create_info{};
                buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffer_create_info.pNext = nullptr;
                buffer_create_info.flags = 0;
                /* We need to inflate the vertex buffer because Vulkan doesn't take per attribute strides. */
                buffer_create_info.size = sizeof ( Vertex ) * triangle_groups[i].mVertexCount;
                buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                buffer_create_info.queueFamilyIndexCount = 0;
                buffer_create_info.pQueueFamilyIndices = nullptr;

                if ( VkResult result = vkCreateBuffer ( mVulkanRenderer->GetDevice(), &buffer_create_info, nullptr, &mBuffers[i].mVertexBuffer ) )
                {
                    std::ostringstream stream;
                    stream << "vkCreateBuffer failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }

                VkMemoryRequirements memory_requirements{};
                vkGetBufferMemoryRequirements ( mVulkanRenderer->GetDevice(), mBuffers[i].mVertexBuffer, &memory_requirements );

                VkMemoryAllocateInfo memory_allocate_info{};
                memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                memory_allocate_info.pNext = nullptr;
                memory_allocate_info.allocationSize = memory_requirements.size;
                memory_allocate_info.memoryTypeIndex = memory_index;

                if ( VkResult result = vkAllocateMemory ( mVulkanRenderer->GetDevice(), &memory_allocate_info, nullptr, &mBuffers[i].mVertexMemory ) )
                {
                    std::ostringstream stream;
                    stream << "vkAllocateMemory failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }

                Vertex* vertices = nullptr;
                if ( VkResult result = vkMapMemory ( mVulkanRenderer->GetDevice(), mBuffers[i].mVertexMemory, 0, VK_WHOLE_SIZE, 0, reinterpret_cast<void**> ( &vertices ) ) )
                {
                    std::ostringstream stream;
                    stream << "vkMapMemory failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }

                memset ( vertices, 0, buffer_create_info.size );
                uintptr_t offset = 0;
                for ( uint32_t j = 0; j < triangle_groups[i].mVertexCount; ++j )
                {
                    if ( triangle_groups[i].mVertexFlags & Mesh::POSITION_BIT )
                    {
                        memcpy ( vertices[j].position, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::position ) );
                        offset += sizeof ( Vertex::position );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::NORMAL_BIT )
                    {
                        memcpy ( vertices[j].normal, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::normal ) );
                        offset += sizeof ( Vertex::normal );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::TANGENT_BIT )
                    {
                        memcpy ( vertices[j].tangent, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::tangent ) );
                        offset += sizeof ( Vertex::tangent );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::BITANGENT_BIT )
                    {
                        memcpy ( vertices[j].bitangent, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::bitangent ) );
                        offset += sizeof ( Vertex::bitangent );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::UV_BIT )
                    {
                        memcpy ( vertices[j].uv, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::uv ) );
                        offset += sizeof ( Vertex::uv );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::WEIGHT_BIT )
                    {
                        memcpy ( vertices[j].weight_indices, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::weight_indices ) );
                        offset += sizeof ( Vertex::weight_indices );
                        memcpy ( vertices[j].weight_influences, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::weight_influences ) );
                        offset += sizeof ( Vertex::weight_influences );
                    }
                }

                vkUnmapMemory ( mVulkanRenderer->GetDevice(), mBuffers[i].mVertexMemory );

                if ( VkResult result = vkBindBufferMemory ( mVulkanRenderer->GetDevice(), mBuffers[i].mVertexBuffer, mBuffers[i].mVertexMemory, 0 ) )
                {
                    std::ostringstream stream;
                    stream << "vkBindBufferMemory failed. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
            if ( triangle_groups[i].mIndexCount )
            {
                VkBufferCreateInfo buffer_create_info{};
                buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffer_create_info.pNext = nullptr;
                buffer_create_info.flags = 0;
                /**@note Upcasting index buffer to 16 bits when index type is 8 bits.*/
                buffer_create_info.size = triangle_groups[i].mIndexBuffer.length() *
                                          ( ( triangle_groups[i].mIndexType == Mesh::BYTE || triangle_groups[i].mIndexType == Mesh::UNSIGNED_BYTE ) ? 2 : 1 );
                buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                buffer_create_info.queueFamilyIndexCount = 0;
                buffer_create_info.pQueueFamilyIndices = nullptr;

                if ( VkResult result = vkCreateBuffer ( mVulkanRenderer->GetDevice(), &buffer_create_info, nullptr, &mBuffers[i].mIndexBuffer ) )
                {
                    std::ostringstream stream;
                    stream << "vkCreateBuffer failed for index buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }

                VkMemoryRequirements memory_requirements{};
                vkGetBufferMemoryRequirements ( mVulkanRenderer->GetDevice(), mBuffers[i].mIndexBuffer, &memory_requirements );

                VkMemoryAllocateInfo memory_allocate_info{};
                memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                memory_allocate_info.pNext = nullptr;
                memory_allocate_info.allocationSize = memory_requirements.size;
                memory_allocate_info.memoryTypeIndex = memory_index;

                if ( VkResult result = vkAllocateMemory ( mVulkanRenderer->GetDevice(), &memory_allocate_info, nullptr, &mBuffers[i].mIndexMemory ) )
                {
                    std::ostringstream stream;
                    stream << "vkAllocateMemory failed for index buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }

                void* data = nullptr;
                if ( VkResult result = vkMapMemory ( mVulkanRenderer->GetDevice(), mBuffers[i].mIndexMemory, 0, VK_WHOLE_SIZE, 0, &data ) )
                {
                    std::ostringstream stream;
                    stream << "vkMapMemory failed for index buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }

                if ( ! ( triangle_groups[i].mIndexType == Mesh::BYTE || triangle_groups[i].mIndexType == Mesh::UNSIGNED_BYTE ) )
                {
                    memcpy ( data, triangle_groups[i].mIndexBuffer.data(), triangle_groups[i].mIndexBuffer.size() );
                }
                else
                {
                    /**@note upcast 8 bit indices.*/
                    for ( size_t j = 0; j < triangle_groups[i].mIndexBuffer.size(); ++j )
                    {
                        reinterpret_cast<uint16_t*> ( data ) [j] = triangle_groups[i].mIndexBuffer[j];
                    }
                }
                vkUnmapMemory ( mVulkanRenderer->GetDevice(), mBuffers[i].mIndexMemory );

                if ( VkResult result = vkBindBufferMemory ( mVulkanRenderer->GetDevice(), mBuffers[i].mIndexBuffer, mBuffers[i].mIndexMemory, 0 ) )
                {
                    std::ostringstream stream;
                    stream << "vkBindBufferMemory failed. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
        }
    }

    void VulkanMesh::Finalize()
    {
        for ( auto& i : mBuffers )
        {
            if ( i.mVertexMemory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVulkanRenderer->GetDevice(), i.mVertexMemory, nullptr );
                i.mVertexMemory = VK_NULL_HANDLE;
            }
            if ( i.mVertexBuffer != VK_NULL_HANDLE )
            {
                vkDestroyBuffer ( mVulkanRenderer->GetDevice(), i.mVertexBuffer, nullptr );
                i.mVertexBuffer = VK_NULL_HANDLE;
            }
            if ( i.mIndexMemory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVulkanRenderer->GetDevice(), i.mIndexMemory, nullptr );
                i.mIndexMemory = VK_NULL_HANDLE;
            }
            if ( i.mIndexBuffer != VK_NULL_HANDLE )
            {
                vkDestroyBuffer ( mVulkanRenderer->GetDevice(), i.mIndexBuffer, nullptr );
                i.mIndexBuffer = VK_NULL_HANDLE;
            }
        }
    }

    VkIndexType VulkanMesh::GetIndexType ( Mesh::IndexType aIndexType ) const
    {
        switch ( aIndexType )
        {
        /**@note BYTE has been upcasted*/
        case Mesh::UNSIGNED_BYTE:
        case Mesh::UNSIGNED_SHORT:
            return VK_INDEX_TYPE_UINT16;
        case Mesh::UNSIGNED_INT:
            return VK_INDEX_TYPE_UINT32;
        default:
            throw std::runtime_error ( "Invalid Index Type." );
        };
        return VK_INDEX_TYPE_MAX_ENUM;
    }
}
