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
#ifndef AEONGAMES_OPENGLSKELETON_H
#define AEONGAMES_OPENGLSKELETON_H
#include "OpenGLFunctions.h"
#include "aeongames/Memory.h"

namespace AeonGames
{
    class Skeleton;
    class OpenGLSkeleton
    {
    public:
        OpenGLSkeleton ( const std::shared_ptr<Skeleton> aSkeleton );
        ~OpenGLSkeleton();
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Skeleton> mSkeleton;
        GLuint mSkeletonBuffer = 0;
    };
}
#endif