#pragma once

#include <jni/types.hpp>
#include <jni/errors.hpp>
#include <jni/functions.hpp>
#include <jni/tagging.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>

#include <exception>
#include <type_traits>

#include <iostream>

namespace jni
   {
    /// Low-level

    template < class M >
    struct NativeMethodMaker
       {
        template < class T >
        struct Maker;

        template < class T, class R, class P, class... Args >
        struct Maker< R (T::*)(JNIEnv*, P*, Args...) const >
           {
            JNINativeMethod< R (JNIEnv*, P*, Args...) >
            operator()(const char* name, const char* sig, const M& m)
               {
                return { name, sig, m };
               }
           };

        template < class T, class R, class P, class... Args >
        struct Maker< R (T::*)(JNIEnv*, P*, Args...) >
           {
            JNINativeMethod< R (JNIEnv*, P*, Args...) >
            operator()(const char* name, const char* sig, const M& m)
               {
                return { name, sig, m };
               }
           };

        auto operator()(const char* name, const char* sig, const M& m)
           {
            return Maker<decltype(&M::operator())>()(name, sig, m);
           }
       };

    template < class R, class P, class... Args >
    struct NativeMethodMaker< R (*)(JNIEnv*, P*, Args...) >
       {
        JNINativeMethod< R (JNIEnv*, P*, Args...) >
        operator()(const char* name, const char* sig, R (*m)(JNIEnv*, P*, Args...))
           {
            return { name, sig, m };
           }
       };

    template < class M >
    auto MakeNativeMethod(const char* name, const char* sig, M&& m)
       {
        return NativeMethodMaker< typename std::decay<M>::type >()(name, sig, std::forward<M>(m));
       }


    /// High-level

    template < class T, class R, class TagType, class... Args >
    struct NativeMethodMaker< R (T::*)(JNIEnv&, Class<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod< UntaggedType<R> (JNIEnv*, jclass*, UntaggedType<Args>...) >
        operator()(const char* name, const M& m)
           {
            static M method(m);

            auto wrapper = [] (JNIEnv* env, jclass* clazz, UntaggedType<Args>... args)
               {
                try
                   {
                    return Untag(method(*env, Class<TagType>(*clazz), Tag<Args>(args)...));
                   }
                catch (...)
                   {
                    ThrowJavaError(*env, std::current_exception());
                    return Untag(R());
                   }
               };

            return { name, TypeSignature<R (Args...)>()(), wrapper };
           }
       };

    template < class T, class TagType, class... Args >
    struct NativeMethodMaker< void (T::*)(JNIEnv&, Class<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod< void (JNIEnv*, jclass*, UntaggedType<Args>...) >
        operator()(const char* name, const M& m)
           {
            static M method(m);

            auto wrapper = [] (JNIEnv* env, jclass* clazz, UntaggedType<Args>... args)
               {
                try
                   {
                    method(*env, Class<TagType>(*clazz), Tag<Args>(args)...);
                   }
                catch (...)
                   {
                    ThrowJavaError(*env, std::current_exception());
                   }
               };

            return { name, TypeSignature<void (Args...)>()(), wrapper };
           }
       };

    template < class T, class R, class TagType, class... Args >
    struct NativeMethodMaker< R (T::*)(JNIEnv&, Object<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod< UntaggedType<R> (JNIEnv*, jobject*, UntaggedType<Args>...) >
        operator()(const char* name, const M& m)
           {
            static M method(m);

            auto wrapper = [] (JNIEnv* env, jobject* obj, UntaggedType<Args>... args)
               {
                try
                   {
                    return Untag(method(*env, Object<TagType>(obj), Tag<Args>(args)...));
                   }
                catch (...)
                   {
                    ThrowJavaError(*env, std::current_exception());
                    return Untag(R());
                   }
               };

            return { name, TypeSignature<R (Args...)>()(), wrapper };
           }
       };

    template < class T, class TagType, class... Args >
    struct NativeMethodMaker< void (T::*)(JNIEnv&, Object<TagType>, Args...) const >
       {
        template < class M >
        JNINativeMethod< void (JNIEnv*, jobject*, UntaggedType<Args>...) >
        operator()(const char* name, const M& m)
           {
            static M method(m);

            auto wrapper = [] (JNIEnv* env, jobject* obj, UntaggedType<Args>... args)
               {
                try
                   {
                    method(*env, Object<TagType>(obj), Tag<Args>(args)...);
                   }
                catch (...)
                   {
                    ThrowJavaError(*env, std::current_exception());
                   }
               };

            return { name, TypeSignature<void (Args...)>()(), wrapper };
           }
       };

    template < class M >
    auto MakeNativeMethod(const char* name, const M& m)
       {
        return NativeMethodMaker<decltype(&M::operator())>()(name, m);
       }


    template < class M, M >
    class NativePeerMethod;

    template < class T, class R, class... Args, R (T::*method)(JNIEnv&, Args...) >
    class NativePeerMethod< R (T::*)(JNIEnv&, Args...), method >
       {
        private:
            const char* name;

        public:
            NativePeerMethod(const char* n)
               : name(n)
               {}

            template < class TagType >
            auto operator()(const Field<TagType, jlong>& field)
               {
                return MakeNativeMethod(name, [&field] (JNIEnv& env, Object<TagType> obj, Args... args)
                   { return (reinterpret_cast<T*>(obj.Get(env, field))->*method)(env, std::move(args)...); });
               }
       };

    /**
     * A registration function for native methods on a "native peer": a long-lived native
     * object corresponding to a Java object, usually created when the Java object is created
     * and destroyed when the Java object's finalizer runs.
     *
     * It assumes that the Java object has a field, named by `fieldName`, of Java type `long`,
     * which is used to hold a pointer to the native peer.
     *
     * `Methods` must be a sequence of `NativePeerMethod` instances, instantiated with pointer
     * to member functions of the native peer class. For each method in `methods`, a native
     * method is bound with a signature corresponding to that of the member function. The
     * wrapper for that native method obtains the native peer instance from the Java field and
     * calls the native peer method on it, passing along any arguments.
     *
     * An overload is provided that accepts a Callable object with a unique_ptr result type and
     * the names for native creation and finalization methods, allowing creation and disposal of
     * the native peer from Java.
     *
     * For an example of all of the above, see the `examples` directory.
     */

    template < class TagType, class... Methods >
    void RegisterNativePeer(JNIEnv& env, const Class<TagType>& clazz, const char* fieldName, Methods&&... methods)
       {
        static Field<TagType, jni::jlong> field { env, clazz, fieldName };
        RegisterNatives(env, clazz, methods(field)...);
       }

    template < class TagType, class Constructor, class... Methods >
    void RegisterNativePeer(JNIEnv& env, const Class<TagType>& clazz, const char* fieldName,
                            Constructor constructor,
                            const char* initializeMethodName,
                            const char* finalizeMethodName,
                            Methods&&... methods)
       {
        using UniquePtr = decltype(constructor());
        using Pointer = typename UniquePtr::pointer;

        static Field<TagType, jlong> field { env, clazz, fieldName };

        auto finalize = [] (JNIEnv& e, Object<TagType> obj)
           {
            UniquePtr instance(reinterpret_cast<Pointer>(obj.Get(e, field)));
            if (instance) obj.Set(e, field, jlong(0));
            instance.reset();
           };

        auto initialize = [constructor] (JNIEnv& e, Object<TagType> obj)
           {
            UniquePtr previous(reinterpret_cast<Pointer>(obj.Get(e, field)));
            UniquePtr instance(constructor());
            obj.Set(e, field, reinterpret_cast<jlong>(instance.get()));
            instance.release();
           };

        RegisterNatives(env, clazz,
            MakeNativeMethod(initializeMethodName, initialize),
            MakeNativeMethod(finalizeMethodName, finalize),
            methods(field)...);
       }
   }
