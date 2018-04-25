/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_FLYWEIGHT_STORE_H
#define AEONGAMES_FLYWEIGHT_STORE_H

#include <unordered_map>
#include <functional>

namespace AeonGames
{
    template<class Key, class Value>
    class FlyWeight
    {
        friend Value;
        FlyWeight() = default;
        FlyWeight ( const Key& aKey ) : mKey ( aKey )
        {
            mStore[mKey] = this;
        }
        virtual ~FlyWeight ()
        {
            mStore.erase ( mKey );
        }
        Key mKey{};
        static std::unordered_map<Key, Value*> mStore;
    };
    template <class Key, class Value>
    std::unordered_map<Key, Value*> FlyWeight<Key, Value>::mStore{};
}
#endif
