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

#ifndef AEONGAMES_PLANE_H
#define AEONGAMES_PLANE_H
/*! \file
    \brief Header for the plane class.
    \author Rodrigo Hernandez.
    \copy 2017
*/

#include "aeongames/Platform.h"
#include "aeongames/Vector3.h"

namespace AeonGames
{
    class AABB;
    /*! \brief Plane class. */
    class Plane
    {
    public:
        ///@brief Default constructor.
        DLL Plane ( float aNormalX, float aNormalY, float aNormalZ, float aDistance );
        /// destructor.
        DLL ~Plane();
        DLL float GetDistanceTo ( const Vector3& aLocation ) const;
        DLL float GetDistanceTo ( const Vector3& aLocation, const AABB& aAABB ) const;
    private:
        Vector3 mNormal;
        float mDistance;
    };
}
#endif
