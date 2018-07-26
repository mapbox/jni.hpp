#pragma once

#include <jni/traits.hpp>

#include <type_traits>

namespace jni
   {
    template < class T >
    auto AddTag(T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T, class U >
    auto AddTag(U* u)
       -> std::enable_if_t< !IsPrimitive<T>::value, T >
       {
        return T(u);
       }

    template < class T, class U >
    auto AddTag(U& u)
       -> std::enable_if_t< !IsPrimitive<T>::value, T >
       {
        return T(u);
       }


    template < class T >
    auto RemoveTag(T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T >
    auto RemoveTag(const T& t)
       -> std::enable_if_t< !IsPrimitive<T>::value, decltype(t.Get()) >
       {
        return t.Get();
       }


    template < class T >
    struct UntaggedTypeTraits
       {
        using Type = decltype(RemoveTag(std::declval<T>()));
       };

    template <>
    struct UntaggedTypeTraits<void>
       {
        using Type = void;
       };

    template < class T >
    using UntaggedType = typename UntaggedTypeTraits<T>::Type;
   }
