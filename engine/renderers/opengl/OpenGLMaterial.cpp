/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include <ostream>
#include <regex>
#include <utility>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ResourceCache.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Material.h"
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"

namespace AeonGames
{
    OpenGLMaterial::OpenGLMaterial ( const Material& aMaterial ) :
        mMaterial ( aMaterial ), mPropertiesBuffer{}
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

    OpenGLMaterial::~OpenGLMaterial()
    {
        Finalize();
    }

    void OpenGLMaterial::Update ( const uint8_t* aValue, size_t aOffset, size_t aSize )
    {
        mPropertiesBuffer.WriteMemory ( aOffset, ( aSize ) ? aSize : mMaterial.GetPropertyBlock().size() - aOffset, aValue );
    }

    const std::vector<std::shared_ptr<OpenGLTexture>>& OpenGLMaterial::GetTextures() const
    {
        return mTextures;
    }

    GLuint OpenGLMaterial::GetPropertiesBufferId() const
    {
        return mPropertiesBuffer.GetBufferId();
    }

    void OpenGLMaterial::Initialize()
    {
        mPropertiesBuffer.Initialize ( static_cast<GLsizei> ( mMaterial.GetPropertyBlock().size() ), GL_DYNAMIC_DRAW, mMaterial.GetPropertyBlock().data() );
    }

    void OpenGLMaterial::Finalize()
    {
        mPropertiesBuffer.Finalize();
    }
}
