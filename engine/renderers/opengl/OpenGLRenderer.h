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
#ifndef AEONGAMES_OPENGLRENDERER_H
#define AEONGAMES_OPENGLRENDERER_H

#include "aeongames/Renderer.h"
#include <exception>
#include <vector>
#include <unordered_map>
#include <utility>
#include "aeongames/Memory.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class Mesh;
    class OpenGLMesh;
    class Program;
    class OpenGLProgram;
    class Material;
    class OpenGLMaterial;
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;
        void BeginRender() const final;
        void EndRender() const final;
        void Render ( const std::shared_ptr<Model> aModel ) const final;
        bool AllocateModelRenderData ( std::shared_ptr<Model> aModel ) final;
        bool AddRenderingWindow ( uintptr_t aWindowId ) final;
        void RemoveRenderingWindow ( uintptr_t aWindowId ) final;
        void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const final;
        void SetViewMatrix ( const float aMatrix[16] ) final;
        void SetProjectionMatrix ( const float aMatrix[16] ) final;
        void SetModelMatrix ( const float aMatrix[16] ) final;
    private:
        void Initialize();
        void Finalize();
        void UpdateMatrices();
        bool AllocateMeshRenderData ( std::shared_ptr<Mesh> aMesh );
        bool AllocateProgramRenderData ( std::shared_ptr<Program> aProgram );
        bool AllocateMaterialRenderData ( std::shared_ptr<Material> aMaterial );
        GLuint mMatricesBuffer = 0;
        float mMatrices[ ( 16 * 6 ) + ( 12 * 1 )] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mViewProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelViewMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelViewProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            /*  mNormalMatrix With Padding,
                this should really be a 3x3 matrix,
                but std140 packing requires 16 byte alignment
                and a mat3 is escentially a vec3[3]*/
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
        };

        float* mViewMatrix = mMatrices + ( 16 * 0 );
        float* mProjectionMatrix = mMatrices + ( 16 * 1 );
        float* mModelMatrix = mMatrices + ( 16 * 2 );
        // Cache Matrices
        float* mViewProjectionMatrix = mMatrices + ( 16 * 3 );
        float* mModelViewMatrix = mMatrices + ( 16 * 4 );
        float* mModelViewProjectionMatrix = mMatrices + ( 16 * 5 );
        float* mNormalMatrix = mMatrices + ( 16 * 6 );

        struct WindowData
        {
            uintptr_t mWindowId = 0;
#ifdef _WIN32
            HDC mDeviceContext = nullptr;
            HGLRC mOpenGLContext = nullptr;
#else
            Display* mDisplay = nullptr;
            GLXContext mOpenGLContext = nullptr;
#endif
        };
        std::vector<WindowData> mWindowRegistry;
        std::unordered_map<std::shared_ptr<Mesh>, std::unique_ptr<OpenGLMesh>> mMeshMap;
        std::unordered_map<std::shared_ptr<Program>, std::unique_ptr<OpenGLProgram>> mProgramMap;
        std::unordered_map<std::shared_ptr<Material>, std::unique_ptr<OpenGLMaterial>> mMaterialMap;
    };
}
#endif
