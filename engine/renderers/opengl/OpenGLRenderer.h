/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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

#include <exception>
#include <vector>
#include <unordered_map>
#include <utility>
#include "aeongames/Memory.h"
#include "aeongames/Renderer.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class OpenGLMesh;
    class OpenGLPipeline;
    class OpenGLModel;
    class OpenGLMaterial;
    class OpenGLRenderer : public Renderer, public std::enable_shared_from_this<OpenGLRenderer>
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;
#if 0
        void BeginRender ( void* aWindowId ) const final;
        void Render ( void* aWindowId, const std::shared_ptr<Model> aModel ) const final;
        void EndRender ( void* aWindowId ) const final;
#endif
        bool AllocateModelRenderData ( std::shared_ptr<Model> aModel ) final;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) final;
#if 0
        bool AddRenderingWindow ( void* aWindowId ) final;
        void RemoveRenderingWindow ( void* aWindowId ) final;
#endif
        void SetViewMatrix ( const float aMatrix[16] ) final;
        void SetProjectionMatrix ( const float aMatrix[16] ) final;
        void SetModelMatrix ( const float aMatrix[16] ) final;
        void* GetOpenGLContext() const;
    private:
        void Initialize();
        void Finalize();
        void UpdateMatrices();
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

        /// Internal Window Id, required to create initial shared context
        void* mWindowId = nullptr;
        /// Internal OpenGL context, shared with all other contexts
        void* mOpenGLContext = nullptr;
        //std::vector<void*> mWindowRegistry;
        std::unordered_map <
        std::shared_ptr<Model>,
            std::shared_ptr<OpenGLModel >>
            mModelMap;
    };
}
#endif
