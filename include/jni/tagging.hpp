#pragma once

#include <jni/traits.hpp>

#include <type_traits>

namespace jni
   {
    template < class T >
    auto Tag(T primitive)
       {
        return primitive;
       }

    template < class T, class U >
    T Tag(U* u)
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
       -> typename std::enable_if< !IsPrimitive<T>::value, typename T::UntaggedType* >::type
       {
        return t.Get();
       }

    template < class T >
    using UntaggedType = decltype(Untag(std::declval<T>()));
   }
