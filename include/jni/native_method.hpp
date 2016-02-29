#pragma once

#include <jni/types.hpp>
#include <jni/errors.hpp>
#include <jni/functions.hpp>
#include <jni/tagging.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>

#include <exception>

namespace jni
   {
    template < class T >
    using NativeType = UnwrappedType<UntaggedType<T>>;

    template < class T >
    struct NativeMethodMaker;

    template < class T, class R, class TagType, class... Args >
    struct NativeMethodMaker< R (T::*)(JNIEnv&, Class<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            NativeType<R> (*wrapper)(JNIEnv*, ::jclass, NativeType<Args>...) =
                [] (JNIEnv* env, ::jclass clazz, NativeType<Args>... args)
                   {
                    try
                       {
                        return Unwrap(Untag(method(*env,
                            Class<TagType>(*Wrap<jclass*>(clazz)),
                            Tag<Args>(Wrap<UntaggedType<Args>>(args))...)));
                       }
                    catch (...)
                       {
                        ThrowJavaError(*env, std::current_exception());
                        return Unwrap(Untag(R()));
                       }
                   };

            return { name, TypeSignature<R (Args...)>()(), reinterpret_cast<void*>(wrapper) };
           }
       };

    template < class T, class TagType, class... Args >
    struct NativeMethodMaker< void (T::*)(JNIEnv&, Class<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            void (*wrapper)(JNIEnv*, ::jclass, NativeType<Args>...) =
                [] (JNIEnv* env, ::jclass clazz, NativeType<Args>... args)
                   {
                    try
                       {
                        method(*env,
                            Class<TagType>(*Wrap<jclass*>(clazz)),
                            Tag<Args>(Wrap<UntaggedType<Args>>(args))...);
                       }
                    catch (...)
                       {
                        ThrowJavaError(*env, std::current_exception());
                       }
                   };

            return { name, TypeSignature<void (Args...)>()(), reinterpret_cast<void*>(wrapper) };
           }
       };

    template < class T, class R, class TagType, class... Args >
    struct NativeMethodMaker< R (T::*)(JNIEnv&, Object<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            NativeType<R> (*wrapper)(JNIEnv*, ::jobject, NativeType<Args>...) =
                [] (JNIEnv* env, ::jobject obj, NativeType<Args>... args)
                   {
                    try
                       {
                        return Unwrap(Untag(method(*env,
                            Object<TagType>(Wrap<jobject*>(obj)),
                            Tag<Args>(Wrap<UntaggedType<Args>>(args))...)));
                       }
                    catch (...)
                       {
                        ThrowJavaError(*env, std::current_exception());
                        return Unwrap(Untag(R()));
                       }
                   };

            return { name, TypeSignature<R (Args...)>()(), reinterpret_cast<void*>(wrapper) };
           }
       };

    template < class T, class TagType, class... Args >
    struct NativeMethodMaker< void (T::*)(JNIEnv&, Object<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            void (*wrapper)(JNIEnv*, ::jobject, NativeType<Args>...) =
                [] (JNIEnv* env, ::jobject obj, NativeType<Args>... args)
                   {
                    try
                       {
                        method(*env,
                            Object<TagType>(Wrap<jobject*>(obj)),
                            Tag<Args>(Wrap<UntaggedType<Args>>(args))...);
                       }
                    catch (...)
                       {
                        ThrowJavaError(*env, std::current_exception());
                       }
                   };

            return { name, TypeSignature<void (Args...)>()(), reinterpret_cast<void*>(wrapper) };
           }
       };

    template < class M >
    JNINativeMethod NativeMethod(const char* name, M&& m)
       {
        return NativeMethodMaker<decltype(&M::operator())>()(name, std::move(m));
       }
   }
