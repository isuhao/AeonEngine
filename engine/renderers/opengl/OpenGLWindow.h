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
#ifndef AEONGAMES_OPENGLWINDOW_H
#define AEONGAMES_OPENGLWINDOW_H

#include <cstdint>
#include <vector>
#include "aeongames/Memory.h"
#include "aeongames/Window.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLWindow : public Window
    {
    public:
        OpenGLWindow ( void* aWindowId, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer );
        virtual ~OpenGLWindow();
        void* GetWindowId() const;
        void ResizeViewport ( uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender() const final;
        void Render ( const std::shared_ptr<RenderModel> aModel ) const final;
        void EndRender() const final;
    private:
        void Initialize();
        void Finalize();
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer;
        void* mWindowId;
    };
}
#endif
