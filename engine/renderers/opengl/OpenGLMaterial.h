/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLMATERIAL_H
#define AEONGAMES_OPENGLMATERIAL_H
#include "aeongames/Material.h"
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace AeonGames
{
    class Texture;
    class Uniform
    {
    public:
        Uniform() {};
        ~Uniform() {};
    private:
        uint32_t mLocation;
        uint32_t mType;
        uint8_t mData[sizeof ( float ) * 4];
        static_assert ( sizeof ( std::shared_ptr<Texture> ) <= ( sizeof ( float ) * 4 ), "Size of shared pointer is bigger than a vec4" );
    };

    class OpenGLMaterial : public Material
    {
    public:
        OpenGLMaterial ( const std::string& aFilename );
        ~OpenGLMaterial();
    private:
        void Initialize();
        void Finalize();
        std::string mFilename;
        std::vector<Uniform> mUniforms;
    };
}
#endif