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
    struct NativeMessageMaker;

    template < class T, class R, class TagType, class... Args >
    struct NativeMessageMaker< R (T::*)(JNIEnv&, Class<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            struct Wrapper
               {
                static NativeType<R>
                Method(JNIEnv* env, ::jclass clazz, NativeType<Args>... args)
                   {
                    try
                       {
                        return Unwrap(Untag(method(*env,
                            Class<TagType>(Wrap<jclass*>(clazz)),
                            Tag<Args>(Wrap<UntaggedType<Args>>(args))...)));
                       }
                    catch (...)
                       {
                        ThrowJavaError(*env, std::current_exception());
                       }

                    return Unwrap(Untag(R()));
                   }
               };

            return { name, TypeSignature<R (Args...)>()(), reinterpret_cast<void*>(&Wrapper::Method) };
           }
       };

    template < class T, class TagType, class... Args >
    struct NativeMessageMaker< void (T::*)(JNIEnv&, Class<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            struct Wrapper
               {
                static void Method(JNIEnv* env, ::jclass clazz, NativeType<Args>... args)
                   {
                    try
                       {
                        method(*env,
                            Class<TagType>(Wrap<jclass*>(clazz)),
                            Tag<Args>(Wrap<UntaggedType<Args>>(args))...);
                       }
                    catch (...)
                       {
                        ThrowJavaError(*env, std::current_exception());
                       }
                   }
               };

            return { name, TypeSignature<void (Args...)>()(), reinterpret_cast<void*>(&Wrapper::Method) };
           }
       };

    template < class T, class R, class TagType, class... Args >
    struct NativeMessageMaker< R (T::*)(JNIEnv&, Object<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            struct Wrapper
               {
                static NativeType<R>
                Method(JNIEnv* env, ::jobject obj, NativeType<Args>... args)
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
                       }

                    return Unwrap(Untag(R()));
                   }
               };

            return { name, TypeSignature<R (Args...)>()(), reinterpret_cast<void*>(&Wrapper::Method) };
           }
       };

    template < class T, class TagType, class... Args >
    struct NativeMessageMaker< void (T::*)(JNIEnv&, Object<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod operator()(const char* name, M&& m)
           {
            static M method = std::move(m);

            struct Wrapper
               {
                static void Method(JNIEnv* env, ::jobject obj, NativeType<Args>... args)
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
                   }
               };

            return { name, TypeSignature<void (Args...)>()(), reinterpret_cast<void*>(&Wrapper::Method) };
           }
       };

    template < class M >
    JNINativeMethod NativeMethod(const char* name, M&& m)
       {
        return NativeMessageMaker<decltype(&M::operator())>()(name, std::move(m));
       }
   }
