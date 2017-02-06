#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>
#include <jni/pointer_to_value.hpp>

#include <cstddef>

namespace jni
   {
    template < class TheTag > class Class;
    template < class TheTag, class > class Field;
    template < class TheTag, class > class Method;

    struct ObjectTag { static constexpr auto Name() { return "java/lang/Object"; } };

    template < class TagType >
    struct UntaggedObjectType { using Type = jobject; };

    template < class TheTag >
    class Object;

    template < class TagType >
    class ObjectDeleter;

    template < class TagType >
    using UniqueObject = std::unique_ptr< const Object<TagType>, ObjectDeleter<TagType> >;

    template < class TagType >
    class WeakObjectRefDeleter;

    template < class TagType >
    using UniqueWeakObject = std::unique_ptr< const Object<TagType>, WeakObjectRefDeleter<TagType> >;

    template < class TheTag = ObjectTag >
    class Object
       {
        public:
            using TagType = TheTag;
            using UntaggedObjectType = typename UntaggedObjectType<TagType>::Type;

        private:
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

            explicit operator bool() const { return obj; }

            operator UntaggedObjectType*() const { return obj; }
            UntaggedObjectType& operator*() const { return *obj; }
            UntaggedObjectType* Get() const { return obj; }

            friend bool operator==( const Object& a, const Object& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const Object& a, const Object& b )  { return !( a == b ); }

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

            UniqueObject<TagType> NewGlobalRef(JNIEnv& env) const
               {
                return Seize(env, Object(jni::NewGlobalRef(env, obj).release()));
               }

            UniqueWeakObject<TagType> NewWeakGlobalRef(JNIEnv& env) const
               {
                return SeizeWeakRef(env, Object(jni::NewWeakGlobalRef(env, obj).release()));
               }

            template < class OtherTag >
            bool IsInstanceOf(JNIEnv& env, const Class<OtherTag>& clazz) const
               {
                return jni::IsInstanceOf(env, obj, clazz);
               }
       };

    template < class TagType >
    class ObjectDeleter
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< Object<TagType> >;

            ObjectDeleter() = default;
            ObjectDeleter(JNIEnv& e) : env(&e) {}

            void operator()(pointer p) const
               {
                if (p)
                   {
                    assert(env);
                    env->DeleteGlobalRef(Unwrap(p->Get()));
                   }
               }
       };

    template < class TagType >
    UniqueObject<TagType> Seize(JNIEnv& env, Object<TagType>&& object)
       {
        return UniqueObject<TagType>(PointerToValue<Object<TagType>>(std::move(object)), ObjectDeleter<TagType>(env));
       };

    template < class TagType >
    class WeakObjectRefDeleter
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< Object<TagType> >;

            WeakObjectRefDeleter() = default;
            WeakObjectRefDeleter(JNIEnv& e) : env(&e) {}

            void operator()(pointer p) const
               {
                if (p)
                   {
                    assert(env);
                    env->DeleteWeakGlobalRef(Unwrap(p->Get()));
                   }
               }
       };

    template < class TagType >
    UniqueWeakObject<TagType> SeizeWeakRef(JNIEnv& env, Object<TagType>&& object)
       {
        return UniqueWeakObject<TagType>(PointerToValue<Object<TagType>>(std::move(object)), WeakObjectRefDeleter<TagType>(env));
       };


    template < class OutTagType, class InTagType >
    Object<OutTagType> Cast(JNIEnv& env, const Object<InTagType>& object, const Class<OutTagType>& clazz)
       {
        if (!object.IsInstanceOf(env, clazz))
           {
            ThrowNew(env, FindClass(env, "java/lang/ClassCastException"));
           }
        return Object<OutTagType>(object.Get());
       }
   }
