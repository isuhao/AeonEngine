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
#ifndef AEONGAMES_WINDOW_H
#define AEONGAMES_WINDOW_H
#include "Memory.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
namespace AeonGames
{
    class Scene;
    class Mesh;
    class Pipeline;
    class Material;
    class Window
    {
    public:
        DLL virtual ~Window() = 0;
        virtual void ResizeViewport ( uint32_t aWidth, uint32_t aHeight ) = 0;
        ///@name Render Functions
        ///@{
        virtual void BeginRender() const = 0;
        virtual void EndRender() const = 0;
        virtual void Render ( const Transform& aModelTransform, const Mesh& aMesh, const Pipeline& aPipeline, const Material* aMaterial = nullptr, uint32_t aVertexStart = 0, uint32_t aVertexCount = 0xffffffff, uint32_t aInstanceCount = 1, uint32_t aFirstInstance = 0 ) const = 0;
        ///@}
        ///@name Matrix Functions
        ///@{
        DLL void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        DLL void SetViewTransform ( const Transform& aTransform );
        DLL const Matrix4x4& GetProjectionMatrix() const;
        DLL const Transform& GetViewTransform() const;
        ///@}
    protected:
        Matrix4x4 mProjectionMatrix{};
        Transform mViewTransform{};
    };
}
#endif
