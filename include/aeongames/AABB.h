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

#ifndef AEONGAMES_AABB_H
#define AEONGAMES_AABB_H
/*! \file
    \brief Header for the axis aligned bounding box class.
    \author Rodrigo Hernandez.
    \copy 2017,2018
*/

#include <array>
#include "aeongames/Platform.h"
#include "aeongames/Vector3.h"

namespace AeonGames
{
    class Transform;
    class Plane;
    /*! \brief Axis Aligned Bounding Box class. */
    class AABB
    {
    public:
        ///@brief Default constructor.
        DLL AABB();
        DLL AABB ( const Vector3& aCenter, const Vector3& aRadii );
        /// destructor.
        DLL ~AABB();
        DLL const Vector3& GetCenter() const;
        DLL const Vector3& GetRadii() const;
        DLL void SetCenter ( const Vector3& aCenter );
        DLL void SetRadii ( const Vector3& aRadii );
        DLL std::array<Vector3, 8> GetPoints ( const Vector3& aOffset = { 0.0f, 0.0f, 0.0f, } ) const;
        /** Returns the shortest distance from any point in the plane's surface
        to the support point of the AABB in the plain normal inverted direction.
        In other words returns the required displacement of the AABB along the normal direction
        to leave the AABB just touching the plane from the positive side.*/
        DLL float GetDistanceToPlane ( const Plane& aPlane ) const;
        /*! \name Operators */
        //@{
        DLL AABB& operator+= ( const AABB& lhs );
        //@}
    private:
        Vector3 mCenter{};
        Vector3 mRadii{};
    };
}
#endif
