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
#ifndef AEONGAMES_WINDOW_H
#define AEONGAMES_WINDOW_H
#include "Memory.h"
namespace AeonGames
{
    class Scene;
    class Window
    {
    public:
        virtual ~Window() {}
        virtual void ResizeViewport ( uint32_t aWidth, uint32_t aHeight ) = 0;
        virtual void BeginRender() const = 0;
        virtual void Render ( const std::shared_ptr<const Scene>& aScene ) const = 0;
        virtual void EndRender() const = 0;
    };
}
#endif
