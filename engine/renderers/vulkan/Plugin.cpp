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
/** \File Implements the interface for the PNG plugin.*/
#include "aeongames/Platform.h"
#include "aeongames/Plugin.h"
#include "VulkanRenderer.h"
#include "aeongames/Memory.h"
#include "glslang/Public/ShaderLang.h"

extern "C"
{
    bool VulkanStartUp()
    {
        glslang::InitializeProcess();
        return AeonGames::RegisterRendererLoader ( "Vulkan",
                [] ()
        {
            return std::make_shared<AeonGames::VulkanRenderer>();
        } );
    }

    void VulkanShutdown()
    {
        AeonGames::UnregisterRendererLoader ( "Vulkan" );
        glslang::FinalizeProcess();
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "Vulkan Renderer",
        "Implements a Vulkan Renderer",
        VulkanStartUp,
        VulkanShutdown
    };
}