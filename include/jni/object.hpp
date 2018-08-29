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
            UntaggedObjectType* obj = nullptr;

        protected:
            explicit Object(std::nullptr_t = nullptr)
               {}

            explicit Object(UntaggedObjectType* o)
               : obj(o)
               {}

            Object(const Object& o)
               : obj(o.obj)
               {}

            template < class Tag >
            Object(const Object<Tag>& o, std::enable_if_t< std::is_convertible<Tag, TagType>::value >* = nullptr)
               : obj(o.Get())
               {}

            ~Object() = default;

            Object& operator=(const Object&) = delete;
            void reset(UntaggedObjectType* o) { obj = o; }

        public:
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
               -> std::enable_if_t< !IsPrimitive<T>::value, Local<T> >
               {
                return Local<T>(env, reinterpret_cast<UntaggedType<T>>(GetField<jobject*>(env, obj, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const Field<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetField<T>(env, obj, field, value);
               }

            template < class Expected, class Actual >
            auto Set(JNIEnv& env, const Field<TagType, Expected>& field, const Actual& value) const
               -> std::enable_if_t< !IsPrimitive<Expected>::value
                                 && std::is_convertible<const Actual&, const Expected&>::value >
               {
                SetField<jobject*>(env, obj, field, value.Get());
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, R >
               {
                return CallMethod<R>(env, obj, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value
                                 && !std::is_void<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, Local<R> >
               {
                return Local<R>(env, reinterpret_cast<UntaggedType<R>>(CallMethod<jobject*>(env, obj, method, Untag(args)...)));
               }

            template < class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const Method<TagType, void (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value >
               {
                CallMethod<void>(env, obj, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, R >
               {
                return CallNonvirtualMethod<R>(env, obj, clazz, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value
                                 && !std::is_void<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, Local<R> >
               {
                return Local<R>(env, reinterpret_cast<UntaggedType<R>>(CallNonvirtualMethod<jobject*>(env, obj, clazz, method, Untag(args)...)));
               }

            template < class... ExpectedArgs, class... ActualArgs >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, void (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value >
               {
                CallNonvirtualMethod<void>(env, obj, clazz, method, Untag(args)...);
               }

            template < template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
            Global<Object<TagType>, Deleter> NewGlobalRef(JNIEnv& env) const
               {
                return Global<Object<TagType>, Deleter>(env, jni::NewGlobalRef(env, obj).release());
               }

            template < template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
            Weak<Object<TagType>, Deleter> NewWeakGlobalRef(JNIEnv& env) const
               {
                return Weak<Object<TagType>, Deleter>(env, jni::NewWeakGlobalRef(env, obj).release());
               }

            Local<Object<TagType>> NewLocalRef(JNIEnv& env) const
               {
                return Local<Object<TagType>>(env, jni::NewLocalRef(env, obj).release());
               }

            template < class OtherTag >
            bool IsInstanceOf(JNIEnv& env, const Class<OtherTag>& clazz) const
               {
                return jni::IsInstanceOf(env, obj, clazz);
               }
       };

    template < class OutTagType, class InTagType >
    Local<Object<OutTagType>> Cast(JNIEnv& env, const Class<OutTagType>& clazz, const Object<InTagType>& object)
       {
        if (!object.IsInstanceOf(env, clazz))
           {
            ThrowNew(env, FindClass(env, "java/lang/ClassCastException"));
           }
        return Local<Object<OutTagType>>(env, reinterpret_cast<UntaggedType<Object<OutTagType>>>(object.NewLocalRef().release()));
       }
   }
