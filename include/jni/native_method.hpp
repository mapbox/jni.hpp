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

    template < class M, class Enable = void >
    struct FunctionTypeTraits;

    template < class R, class... Args >
    struct FunctionTypeTraits< R (Args...) >
       {
        using Type = R (Args...);
        using ResultType = R;
       };

    template < class R, class... Args >
    struct FunctionTypeTraits< R (*)(Args...) >
        : FunctionTypeTraits< R (Args...) > {};

    template < class T, class R, class... Args >
    struct FunctionTypeTraits< R (T::*)(Args...) const >
        : FunctionTypeTraits< R (Args...) > {};

    template < class T, class R, class... Args >
    struct FunctionTypeTraits< R (T::*)(Args...) >
        : FunctionTypeTraits< R (Args...) > {};

    template < class M >
    struct FunctionTypeTraits< M, typename std::enable_if< std::is_class<M>::value >::type >
        : FunctionTypeTraits< decltype(&M::operator()) > {};

    template < class M >
    auto MakeNativeMethod(const char* name, const char* sig, const M& m)
       {
        using FunctionType = typename FunctionTypeTraits<M>::Type;
        using ResultType = typename FunctionTypeTraits<M>::ResultType;

        static FunctionType* method = m;

        auto wrapper = [] (JNIEnv* env, auto... args)
           {
            try
               {
                return method(env, args...);
               }
            catch (...)
               {
                ThrowJavaError(*env, std::current_exception());
                return ResultType();
               }
           };

        return JNINativeMethod< FunctionType > { name, sig, wrapper };
       }

    template < class M, M method >
    auto MakeNativeMethod(const char* name, const char* sig)
       {
        using FunctionType = typename FunctionTypeTraits<M>::Type;
        using ResultType = typename FunctionTypeTraits<M>::ResultType;

        auto wrapper = [] (JNIEnv* env, auto... args)
           {
            try
               {
                return method(env, args...);
               }
            catch (...)
               {
                ThrowJavaError(*env, std::current_exception());
                return ResultType();
               }
           };

        return JNINativeMethod< FunctionType > { name, sig, wrapper };
       }


    /// High-level

    template < class T, T... >
    struct NativeMethodMaker;

    template < class T, class R, class Subject, class... Args >
    struct NativeMethodMaker< R (T::*)(JNIEnv&, Subject, Args...) const >
       {
        template < class M >
        auto operator()(const char* name, const M& m)
           {
            static M method(m);

            auto wrapper = [] (JNIEnv* env, UntaggedType<Subject> subject, UntaggedType<Args>... args)
               {
                return Untag(method(*env, Tag<Subject>(*subject), Tag<Args>(args)...));
               };

            return MakeNativeMethod(name, TypeSignature<R (Args...)>()(), wrapper);
           }
       };

    template < class T, class Subject, class... Args >
    struct NativeMethodMaker< void (T::*)(JNIEnv&, Subject, Args...) const >
       {
        template < class M >
        auto operator()(const char* name, const M& m)
           {
            static M method(m);

            auto wrapper = [] (JNIEnv* env, UntaggedType<Subject> subject, UntaggedType<Args>... args)
               {
                method(*env, Tag<Subject>(*subject), Tag<Args>(args)...);
               };

            return MakeNativeMethod(name, TypeSignature<void (Args...)>()(), wrapper);
           }
       };

    template < class M >
    auto MakeNativeMethod(const char* name, const M& m)
       {
        return NativeMethodMaker<decltype(&M::operator())>()(name, m);
       }

    template < class R, class Subject, class... Args, R (*method)(JNIEnv&, Subject, Args...) >
    struct NativeMethodMaker< R (JNIEnv&, Subject, Args...), method >
       {
        auto operator()(const char* name)
           {
            auto wrapper = [] (JNIEnv* env, UntaggedType<Subject> subject, UntaggedType<Args>... args)
               {
                return Untag(method(*env, Tag<Subject>(*subject), Tag<Args>(args)...));
               };

            return MakeNativeMethod(name, TypeSignature<R (Args...)>()(), wrapper);
           }
       };

    template < class Subject, class... Args, void (*method)(JNIEnv&, Subject, Args...) >
    struct NativeMethodMaker< void (JNIEnv&, Subject, Args...), method >
       {
        auto operator()(const char* name)
           {
            auto wrapper = [] (JNIEnv* env, UntaggedType<Subject> subject, UntaggedType<Args>... args)
               {
                method(*env, Tag<Subject>(*subject), Tag<Args>(args)...);
               };

            return MakeNativeMethod(name, TypeSignature<void (Args...)>()(), wrapper);
           }
       };

    template < class M, M method >
    auto MakeNativeMethod(const char* name)
       {
        using FunctionType = typename FunctionTypeTraits<M>::Type;
        return NativeMethodMaker<FunctionType, method>()(name);
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
