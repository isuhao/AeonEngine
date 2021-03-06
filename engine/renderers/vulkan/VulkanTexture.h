/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANTEXTURE_H
#define AEONGAMES_VULKANTEXTURE_H
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "aeongames/Memory.h"

namespace AeonGames
{
    class Image;
    class VulkanRenderer;
    class VulkanTexture
    {
    public:
        VulkanTexture ( const std::shared_ptr<const Image>&  aImage, const std::shared_ptr<const VulkanRenderer>&  aVulkanRenderer );
        ~VulkanTexture();
        const VkDescriptorImageInfo& GetDescriptorImageInfo() const;
    private:
        void Initialize();
        void Finalize();
        void InitializeImage();
        void FinalizeImage();
        void InitializeImageView();
        void FinalizeImageView();
        std::shared_ptr<const VulkanRenderer> mVulkanRenderer;
        std::shared_ptr<const Image> mImage;
        VkImage mVkImage;
        VkDeviceMemory mImageMemory;
        VkDescriptorImageInfo mVkDescriptorImageInfo;
    };
}
#endif
