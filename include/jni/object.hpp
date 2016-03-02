#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>

#include <cstddef>

namespace jni
   {
    template < class TheTag > class Class;
    template < class TheTag, class > class Field;
    template < class TheTag, class > class Method;

    struct ObjectTag { static constexpr auto Name() { return "java/lang/Object"; } };

    template < class TagType >
    struct UntaggedObjectType { using Type = jobject; };

    template < class TheTag = ObjectTag >
    class Object
       {
        public:
            using TagType = TheTag;
            using UntaggedObjectType = typename UntaggedObjectType<TagType>::Type;

        private:
            UniqueGlobalRef<UntaggedObjectType> reference;
            UntaggedObjectType* obj = nullptr;

        public:
            explicit Object(std::nullptr_t = nullptr)
               {}

            explicit Object(UntaggedObjectType* o)
               : obj(o)
               {}

            explicit Object(UntaggedObjectType& o)
               : obj(&o)
               {}

            explicit Object(UniqueGlobalRef<UntaggedObjectType>&& r)
               : reference(std::move(r)),
                 obj(reference.get())
               {}

            explicit operator bool() const { return obj; }
            UntaggedObjectType& operator*() const { return *obj; }
            UntaggedObjectType* Get() const { return obj; }

            template < class T >
            auto Get(JNIEnv& env, const Field<TagType, T>& field) const
               -> std::enable_if_t< IsPrimitive<T>::value, T >
               {
                return GetField<T>(env, obj, field);
               }

            template < class T >
            auto Get(JNIEnv& env, const Field<TagType, T>& field) const
               -> std::enable_if_t< !IsPrimitive<T>::value, T >
               {
                return T(reinterpret_cast<UntaggedType<T>>(GetField<jobject*>(env, obj, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const Field<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetField<T>(env, obj, field, value);
               }

            template < class T >
            auto Set(JNIEnv& env, const Field<TagType, T>& field, const T& value) const
               -> std::enable_if_t< !IsPrimitive<T>::value >
               {
                SetField<jobject*>(env, obj, field, value.Get());
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const Method<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value, R >
               {
                return CallMethod<R>(env, obj, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const Method<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value, R >
               {
                return R(reinterpret_cast<UntaggedType<R>>(CallMethod<jobject*>(env, obj, method, Untag(args)...)));
               }

            template < class... Args >
            void Call(JNIEnv& env, const Method<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallMethod<void>(env, obj, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value, R >
               {
                return CallNonvirtualMethod<R>(env, obj, clazz, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value, R >
               {
                return R(reinterpret_cast<UntaggedType<R>>(CallNonvirtualMethod<jobject*>(env, obj, clazz, method, Untag(args)...)));
               }

            template < class... Args >
            void CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallNonvirtualMethod<void>(env, obj, clazz, method, Untag(args)...);
               }

            Object NewGlobalRef(JNIEnv& env) const
               {
                return Object(jni::NewGlobalRef(env, obj));
               }

            Object Release()
               {
                return Object(reference.release());
               }
       };
   }
