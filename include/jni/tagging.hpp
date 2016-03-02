#pragma once

#include <jni/traits.hpp>

#include <type_traits>

namespace jni
   {
    template < class T >
    auto Tag(T primitive)
       -> typename std::enable_if< IsPrimitive<T>::value, T >::type
       {
        return primitive;
       }

    template < class T, class U >
    auto Tag(U* u)
       -> typename std::enable_if< !IsPrimitive<T>::value, T >::type
       {
        return T(u);
       }

    template < class T, class U >
    auto Tag(U& u)
       -> typename std::enable_if< !IsPrimitive<T>::value, T >::type
       {
        return T(u);
       }


    template < class T >
    auto Untag(T primitive)
       -> typename std::enable_if< IsPrimitive<T>::value, T >::type
       {
        return primitive;
       }

    template < class T >
    auto Untag(const T& t)
       -> typename std::enable_if< !IsPrimitive<T>::value, decltype(t.Get()) >::type
       {
        return t.Get();
       }

    template < class T >
    using UntaggedType = decltype(Untag(std::declval<T>()));
   }
