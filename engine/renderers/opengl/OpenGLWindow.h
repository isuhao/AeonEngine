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
#ifndef AEONGAMES_OPENGLWINDOW_H
#define AEONGAMES_OPENGLWINDOW_H

#include <cstdint>
#include <vector>
#include <mutex>
#include "aeongames/Memory.h"
#include "aeongames/Window.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLWindow : public Window
    {
    public:
        OpenGLWindow ( void* aWindowId, const std::shared_ptr<const OpenGLRenderer>&  aOpenGLRenderer );
        ~OpenGLWindow() final;
        void* GetWindowId() const;
        void ResizeViewport ( uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender() const final;
        void EndRender() const final;
        void Render (   const Transform& aModelTransform,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const final;
        const GLuint GetMatricesBuffer() const;
    private:
        void Initialize();
        void Finalize();
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer;
        void* mWindowId{};
        mutable void* mDeviceContext{};
        GLuint mMatricesBuffer{};
    };
}
#endif
