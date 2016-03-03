#pragma once

#include <jni/traits.hpp>

#include <type_traits>

namespace jni
   {
    template < class T >
    auto Tag(T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T, class U >
    auto Tag(U* u)
       -> std::enable_if_t< !IsPrimitive<T>::value, T >
       {
        return T(u);
       }

    template < class T, class U >
    auto Tag(U& u)
       -> std::enable_if_t< !IsPrimitive<T>::value, T >
       {
        return T(u);
       }


    template < class T >
    auto Untag(T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T >
    auto Untag(const T& t)
       -> std::enable_if_t< !IsPrimitive<T>::value, decltype(t.Get()) >
       {
        return t.Get();
       }


    template < class T >
    struct UntaggedTypeTraits
       {
        using Type = decltype(Untag(std::declval<T>()));
       };

    template <>
    struct UntaggedTypeTraits<void>
       {
        using Type = void;
       };

    template < class T >
    using UntaggedType = typename UntaggedTypeTraits<T>::Type;
   }
