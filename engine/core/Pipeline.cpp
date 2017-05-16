/*
Copyright (C) 2017-2016 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include <regex>
#include <array>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#include "aeongames/Utilities.h"
#include "aeongames/Pipeline.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    static const std::array<const char*, 7> AttributeStrings
    {
        "VertexPosition",
        "VertexNormal",
        "VertexTangent",
        "VertexBitangent",
        "VertexUV",
        "VertexWeightIndices",
        "VertexWeights"
    };

    static const std::array<const char*, 7> AttributeTypes
    {
        "vec3",
        "vec3",
        "vec3",
        "vec3",
        "vec2",
        "vec4",
        "vec4"
    };

    Pipeline::Pipeline ( std::string  aFilename ) :
        mFilename ( std::move ( aFilename ) ), mAttributes ( 0 ), mVertexShader(), mFragmentShader()
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

    Pipeline::~Pipeline()
        = default;

    const std::string & Pipeline::GetVertexShaderSource() const
    {
        return mVertexShader;
    }

    const std::string & Pipeline::GetFragmentShaderSource() const
    {
        return mFragmentShader;
    }

    const std::vector<Uniform>& Pipeline::GetUniformMetaData() const
    {
        return mUniformMetaData;
    }

    uint32_t Pipeline::GetAttributes() const
    {
        return mAttributes;
    }

    uint32_t Pipeline::GetLocation ( AttributeBits aAttributeBit ) const
    {
        return ffs ( aAttributeBit );
    }

    uint32_t Pipeline::GetStride() const
    {
        uint32_t stride = 0;
        for ( uint32_t i = 0; i < ffs ( ~VertexAllBits ); ++i )
        {
            if ( mAttributes & ( 1 << i ) )
            {
                stride += GetSize ( static_cast<AttributeBits> ( 1 << i ) );
            }
        }
        return stride;
    }

    Pipeline::AttributeFormat Pipeline::GetFormat ( AttributeBits aAttributeBit ) const
    {
        return ( aAttributeBit & VertexUVBit ) ? Vector2Float :
               ( aAttributeBit & ( VertexWeightIndicesBit | VertexWeightsBit ) ) ? Vector4Byte : Vector3Float;
    }

    uint32_t Pipeline::GetSize ( AttributeBits aAttributeBit ) const
    {
        switch ( GetFormat ( aAttributeBit ) )
        {
        case Vector2Float:
            return sizeof ( float ) * 2;
        case Vector3Float:
            return sizeof ( float ) * 3;
        case Vector4Byte:
            return sizeof ( uint8_t ) * 4;
        }
        return 0;
    }

    uint32_t Pipeline::GetOffset ( AttributeBits aAttributeBit ) const
    {
        uint32_t offset = 0;
        for ( uint32_t i = 1; i != aAttributeBit; i = i << 1 )
        {
            if ( i & mAttributes )
            {
                offset += GetSize ( static_cast<AttributeBits> ( i ) );
            }
        }
        return offset;
    }

    uint32_t Pipeline::GetUniformBlockSize() const
    {
        uint32_t size = 0;
        for ( auto& i : mUniformMetaData )
        {
            switch ( i.GetType() )
            {
            case Uniform::FLOAT:
                size += sizeof ( float );
                break;
            case Uniform::FLOAT_VEC2:
                size += sizeof ( float ) * 2;
                break;
            case Uniform::FLOAT_VEC3:
                size += sizeof ( float ) * 3;
                break;
            case Uniform::FLOAT_VEC4:
                size += sizeof ( float ) * 4;
                break;
            case Uniform::SAMPLER_2D:
                break;
            case Uniform::SAMPLER_CUBE:
                break;
            default:
                break;
            }
        }
        return size;
    }

    void Pipeline::Initialize()
    {
        static PipelineBuffer pipeline_buffer;
        LoadProtoBufObject<PipelineBuffer> ( pipeline_buffer, mFilename, "AEONPRG" );
        {
            mVertexShader.append ( "#version " + std::to_string ( pipeline_buffer.glsl_version() ) + "\n" );
            mVertexShader.append (
                "layout(binding = 0,std140) uniform Matrices{\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n"
            );

            /* Find out which attributes are being used and add them to the shader source */
            std::smatch attribute_matches;
            /**@note static const regex: construct once, use for ever.*/
            static const std::regex attribute_regex (
                "\\bVertexPosition\\b|"
                "\\bVertexNormal\\b|"
                "\\bVertexTangent\\b|"
                "\\bVertexBitangent\\b|"
                "\\bVertexUV\\b|"
                "\\bVertexWeightIndices\\b|"
                "\\bVertexWeights\\b" );
            std::string code = pipeline_buffer.vertex_shader().code();
            while ( std::regex_search ( code, attribute_matches, attribute_regex ) )
            {
                std::cout << attribute_matches.str() << std::endl;
                for ( uint32_t i = 0; i < AttributeStrings.size(); ++i )
                {
                    if ( attribute_matches.str().substr ( 6 ) == AttributeStrings[i] + 6 )
                    {
                        if ( ! ( mAttributes & ( 1 << i ) ) )
                        {
                            mAttributes |= ( 1 << i );
                            mVertexShader.append ( "layout(location = " );
                            mVertexShader.append ( std::to_string ( i ) );
                            mVertexShader.append ( ") in " );
                            mVertexShader.append ( AttributeTypes[i] );
                            mVertexShader.append ( " " );
                            mVertexShader.append ( AttributeStrings[i] );
                            mVertexShader.append ( ";\n" );
                        }
                        break;
                    }
                }
                code = attribute_matches.suffix();
            }

            mFragmentShader.append ( "#version " + std::to_string ( pipeline_buffer.glsl_version() ) + "\n" );
            mFragmentShader.append (
                "layout(binding = 0,std140) uniform Matrices{\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n" );
            mUniformMetaData.clear();
            mUniformMetaData.reserve ( pipeline_buffer.property().size() );
            if ( pipeline_buffer.property().size() > 0 )
            {
                std::string properties ( "layout(std140) uniform Properties{\n" );
                std::string samplers ( "//----SAMPLERS-START----\n" );
                for ( auto& i : pipeline_buffer.property() )
                {
                    switch ( i.default_value_case() )
                    {
                    case PropertyBuffer::DefaultValueCase::kScalarFloat:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.scalar_float() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer::DefaultValueCase::kVector2:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector2().x(), i.vector2().y() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer::DefaultValueCase::kVector3:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector3().x(), i.vector3().y(), i.vector3().z() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer::DefaultValueCase::kVector4:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector4().x(), i.vector4().y(), i.vector4().z(), i.vector4().w() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer::DefaultValueCase::kTexture:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.texture() );
                        samplers.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
#if 0
                    case PropertyBuffer_Type_SAMPLER_CUBE:
                        //type_name = "samplerCube ";
                        /* To be continued ... */
                        break;
#endif
                    default:
                        throw std::runtime_error ( "Unknown Type." );
                    }
                }
                properties.append ( "};\n" );
                samplers.append ( "//----SAMPLERS-END----\n" );
                mVertexShader.append ( properties );
                mVertexShader.append ( samplers );
                mFragmentShader.append ( properties );
                mFragmentShader.append ( samplers );
            }
            switch ( pipeline_buffer.vertex_shader().source_case() )
            {
            case ShaderBuffer::SourceCase::kCode:
            {
                mVertexShader.append ( pipeline_buffer.vertex_shader().code() );
            }
            break;
            default:
                throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
            }
            switch ( pipeline_buffer.fragment_shader().source_case() )
            {
            case ShaderBuffer::SourceCase::kCode:
                mFragmentShader.append ( pipeline_buffer.fragment_shader().code() );
                break;
            default:
                throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
            }
        }
        pipeline_buffer.Clear();
#if 0
        std::ofstream shader_vert ( "shader.vert" );
        std::ofstream shader_frag ( "shader.frag" );
        shader_vert << mVertexShader;
        shader_frag << mFragmentShader;
        shader_vert.close();
        shader_frag.close();
#endif
    }

    void Pipeline::Finalize()
    {
    }
}