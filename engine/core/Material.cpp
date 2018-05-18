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
#include "aeongames/ProtoBufClasses.h"
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

namespace AeonGames
{
    Material::Material() = default;

    Material::Material ( const std::string&  aFilename )
    {
        try
        {
            Load ( aFilename );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Material::Material ( const void* aBuffer, size_t aBufferSize )
    {
        try
        {
            Load ( aBuffer, aBufferSize );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Material::Material ( const MaterialBuffer & aMaterialBuffer )
    {
        try
        {
            Load ( aMaterialBuffer );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Material::Material ( const Material& aMaterial ) :
        mUniforms ( aMaterial.mUniforms ),
        mUniformBlock ( aMaterial.mUniformBlock ),
        mRenderMaterial{}
    {
    }

    Material& Material::operator = ( const Material& aMaterial )
    {
        // This will not Work
        mUniforms = aMaterial.mUniforms;
        mUniformBlock = aMaterial.mUniformBlock;
        mRenderMaterial.reset();
        return *this;
    }

    Material::~Material()
        = default;

    void Material::Load ( const std::string&  aFilename )
    {
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aFilename, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }

    void Material::Load ( const void* aBuffer, size_t aBufferSize )
    {
        Unload();
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aBuffer, aBufferSize, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }

    static size_t GetUniformBlockSize ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = 0;
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.default_value_case() )
            {
            case PropertyBuffer::DefaultValueCase::kScalarFloat:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                size += sizeof ( float );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarUint:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                size += sizeof ( uint32_t );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarInt:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to uint
                size += sizeof ( int32_t );
                break;
            case PropertyBuffer::DefaultValueCase::kVector2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                size += sizeof ( float ) * 2;
                break;
            case PropertyBuffer::DefaultValueCase::kVector3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 3;
                break;
            case PropertyBuffer::DefaultValueCase::kVector4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
        }
        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats
        return size;
    }

    void Material::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        Unload();
        mUniforms.reserve ( aMaterialBuffer.property().size() );
        mUniformBlock.resize ( AeonGames::GetUniformBlockSize ( aMaterialBuffer ) );
        size_t offset = 0;
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.default_value_case() )
            {
            case PropertyBuffer::DefaultValueCase::kScalarFloat:
                offset += ( offset % sizeof ( float ) ) ? sizeof ( float ) - ( offset % sizeof ( float ) ) : 0;
                mUniforms.emplace_back ( i.uniform_name(), i.scalar_float(), mUniformBlock.data() + offset );
                offset += sizeof ( float );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarUint:
                offset += ( offset % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( offset % sizeof ( uint32_t ) ) : 0;
                mUniforms.emplace_back ( i.uniform_name(), i.scalar_uint(), mUniformBlock.data() + offset );
                offset += sizeof ( uint32_t );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarInt:
                offset += ( offset % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( offset % sizeof ( int32_t ) ) : 0;
                mUniforms.emplace_back ( i.uniform_name(), i.scalar_int(), mUniformBlock.data() + offset );
                offset += sizeof ( int32_t );
                break;
            case PropertyBuffer::DefaultValueCase::kVector2:
                offset += ( offset % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( offset % ( sizeof ( float ) * 2 ) ) : 0;
                mUniforms.emplace_back ( i.uniform_name(), i.vector2().x(), i.vector2().y(), mUniformBlock.data() + offset );
                offset += ( sizeof ( float ) * 2 );
                break;
            case PropertyBuffer::DefaultValueCase::kVector3:
                offset += ( offset % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( offset % ( sizeof ( float ) * 4 ) ) : 0;
                mUniforms.emplace_back ( i.uniform_name(), i.vector3().x(), i.vector3().y(), i.vector3().z(), mUniformBlock.data() + offset );
                offset += ( sizeof ( float ) * 3 );
                break;
            case PropertyBuffer::DefaultValueCase::kVector4:
                offset += ( offset % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( offset % ( sizeof ( float ) * 4 ) ) : 0;
                mUniforms.emplace_back ( i.uniform_name(), i.vector4().x(), i.vector4().y(), i.vector4().z(), i.vector4().w(), mUniformBlock.data() + offset );
                offset += ( sizeof ( float ) * 4 );
                break;
            case PropertyBuffer::DefaultValueCase::kTexture:
                mUniforms.emplace_back ( i.uniform_name(), i.texture() );
                break;
            default:
                throw std::runtime_error ( "Unknown Type." );
            }
        }
    }

    void Material::Unload()
    {
        mUniforms.clear();
    }

    const std::vector<Material::Uniform>& Material::GetUniforms() const
    {
        return mUniforms;
    }

    const std::vector<uint8_t>& Material::GetUniformBlock() const
    {
        return mUniformBlock;
    }
    void Material::SetUniform ( const std::string& aName, void* aValue )
    {
        auto i = std::find_if ( mUniforms.begin(), mUniforms.end(), [&aName] ( const Uniform & aUniform )
        {
            return aUniform.GetName() == aName;
        } );
        if ( i != mUniforms.end() )
        {
            ( *i ).Set ( aValue );
            if ( mRenderMaterial )
            {
                mRenderMaterial->Update ( mUniformBlock.data() );
            }
        }
    }
    void Material::SetRenderMaterial ( std::unique_ptr<IRenderMaterial> aRenderMaterial ) const
    {
        mRenderMaterial = std::move ( aRenderMaterial );
    }

    const Material::IRenderMaterial* const Material::GetRenderMaterial() const
    {
        return mRenderMaterial.get();
    }

    Material::Uniform::Uniform ( const std::string& aName, float aX, uint8_t* aData ) :
        mName ( aName ),
        mType ( Material::Uniform::Type::FLOAT ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
    }
    Material::Uniform::Uniform ( const std::string & aName, uint32_t aX, uint8_t* aData ) :
        mName{aName},
        mType ( Material::Uniform::Type::UINT ),
        mData{ aData }
    {
        ( reinterpret_cast<uint32_t*> ( mData ) ) [0] = aX;
    }
    Material::Uniform::Uniform ( const std::string & aName, int32_t aX, uint8_t* aData ) :
        mName{ aName },
        mType ( Material::Uniform::Type::SINT ),
        mData{ aData }
    {
        ( reinterpret_cast<int32_t*> ( mData ) ) [0] = aX;
    }
    Material::Uniform::Uniform ( const std::string&  aName, float aX, float aY, uint8_t* aData ) :
        mName ( aName ),
        mType ( Material::Uniform::Type::FLOAT_VEC2 ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
    }
    Material::Uniform::Uniform ( const std::string&  aName, float aX, float aY, float aZ, uint8_t* aData ) :
        mName ( aName ),
        mType ( Material::Uniform::Type::FLOAT_VEC3 ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
    }
    Material::Uniform::Uniform ( const std::string&  aName, float aX, float aY, float aZ, float aW, uint8_t* aData ) :
        mName ( aName ),
        mType ( Material::Uniform::Type::FLOAT_VEC4 ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = aW;
    }
    Material::Uniform::Uniform ( const std::string&  aName, const std::string & aFilename ) :
        mName ( aName ),
        mType ( Material::Uniform::Type::SAMPLER_2D ),
        mData{ nullptr }
    {
#if 0
        /// To be refactored
        /**@todo Temporarily hardcoding ".png" identifier,
        the AeonGames::GetImage has to change to deduce image type based on extension.*/
        new ( mData ) std::shared_ptr<Image> ( AeonGames::GetImage ( ".png", aFilename ) );
#endif
    }
    Material::Uniform::~Uniform()
    {
        switch ( mType )
        {
        case Material::Uniform::Type::SAMPLER_2D:
            reinterpret_cast<std::shared_ptr<Image>*> ( mData )->~shared_ptr();
            break;
        default:
            break;
        }
    }

    Material::Uniform::Type Material::Uniform::GetType() const
    {
        return mType;
    }

    const std::string Material::Uniform::GetDeclaration() const
    {
        std::string declaration;
        switch ( mType )
        {
        case Material::Uniform::Type::FLOAT:
            declaration = "float " + mName + ";\n";
            break;
        case Material::Uniform::Type::UINT:
            declaration = "uint " + mName + ";\n";
            break;
        case Material::Uniform::Type::SINT:
            declaration = "int " + mName + ";\n";
            break;
        case Material::Uniform::Type::FLOAT_VEC2:
            declaration = "vec2 " + mName + ";\n";
            break;
        case Material::Uniform::Type::FLOAT_VEC3:
            declaration = "vec3 " + mName + ";\n";
            break;
        case Material::Uniform::Type::FLOAT_VEC4:
            declaration = "vec4 " + mName + ";\n";
            break;
        case Material::Uniform::Type::SAMPLER_2D:
            declaration = "uniform sampler2D " + mName + ";\n";
            break;
        default:
            break;
        }
        return declaration;
    }

    const std::string & Material::Uniform::GetName() const
    {
        return mName;
    }

    uint32_t Material::Uniform::GetUInt() const
    {
        assert ( mType == Material::Uniform::Type::UINT );
        return ( reinterpret_cast<const uint32_t*> ( mData ) ) [0];
    }

    DLL int32_t Material::Uniform::GetSInt() const
    {
        assert ( mType == Material::Uniform::Type::SINT );
        return ( reinterpret_cast<const int32_t*> ( mData ) ) [0];
    }

    float Material::Uniform::GetX() const
    {
        assert (
            mType == Material::Uniform::Type::FLOAT ||
            mType == Material::Uniform::Type::FLOAT_VEC2 ||
            mType == Material::Uniform::Type::FLOAT_VEC3 ||
            mType == Material::Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [0];
    }
    float Material::Uniform::GetY() const
    {
        assert ( mType == Material::Uniform::Type::FLOAT_VEC2 ||
                 mType == Material::Uniform::Type::FLOAT_VEC3 ||
                 mType == Material::Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [1];
    }
    float Material::Uniform::GetZ() const
    {
        assert ( mType == Material::Uniform::Type::FLOAT_VEC3 ||
                 mType == Material::Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [2];
    }
    float Material::Uniform::GetW() const
    {
        assert ( mType == Material::Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [3];
    }
    const std::shared_ptr<Image> Material::Uniform::GetImage() const
    {
        assert ( mType == Material::Uniform::Type::SAMPLER_2D );
        return * ( reinterpret_cast<const std::shared_ptr<Image>*> ( mData ) );
    }

    void Material::Uniform::SetUInt ( uint32_t aValue )
    {
        assert ( mType == Material::Uniform::Type::UINT );
        ( reinterpret_cast<uint32_t*> ( mData ) ) [0] = aValue;
    }
    void Material::Uniform::SetSInt ( int32_t aValue )
    {
        assert ( mType == Material::Uniform::Type::SINT );
        ( reinterpret_cast<int32_t*> ( mData ) ) [0] = aValue;
    }
    void Material::Uniform::SetX ( float aValue )
    {
        assert (
            mType == Material::Uniform::Type::FLOAT ||
            mType == Material::Uniform::Type::FLOAT_VEC2 ||
            mType == Material::Uniform::Type::FLOAT_VEC3 ||
            mType == Material::Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [0] = aValue;
    }
    void Material::Uniform::SetY ( float aValue )
    {
        assert (
            mType == Material::Uniform::Type::FLOAT_VEC2 ||
            mType == Material::Uniform::Type::FLOAT_VEC3 ||
            mType == Material::Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [1] = aValue;
    }
    void Material::Uniform::SetZ ( float aValue )
    {
        assert (
            mType == Material::Uniform::Type::FLOAT_VEC3 ||
            mType == Material::Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [2] = aValue;
    }
    void Material::Uniform::SetW ( float aValue )
    {
        assert ( mType == Material::Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [3] = aValue;
    }
    void Material::Uniform::Set ( void* aValue )
    {
        switch ( mType )
        {
        case UINT:
            memcpy ( mData, aValue, sizeof ( uint32_t ) );
            break;
        case FLOAT:
            memcpy ( mData, aValue, sizeof ( float ) );
            break;
        case SINT:
            memcpy ( mData, aValue, sizeof ( int32_t ) );
            break;
        case FLOAT_VEC2:
            memcpy ( mData, aValue, sizeof ( float ) * 2 );
            break;
        case FLOAT_VEC3:
            memcpy ( mData, aValue, sizeof ( float ) * 3 );
            break;
        case FLOAT_VEC4:
            memcpy ( mData, aValue, sizeof ( float ) * 4 );
            break;
        default:
            // Do nothing for now.
            break;
        }
    }

}
