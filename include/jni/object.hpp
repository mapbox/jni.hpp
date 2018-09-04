#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>

#include <cstddef>

namespace jni
   {
    template < class TheTag > class Class;
    template < class TheTag, class > class Field;
    template < class TheTag, class > class Method;

    struct ObjectTag
       {
        static constexpr auto Name() { return "java/lang/Object"; }
       };

    struct StringTag
       {
        using SuperTag = ObjectTag;
        static constexpr auto Name() { return "java/lang/String"; }
       };

    class ObjectBase
       {
        protected:
            jobject* ptr = nullptr;

            explicit ObjectBase(std::nullptr_t = nullptr)
               {}

            explicit ObjectBase(jobject* p)
               : ptr(p)
               {}

            ~ObjectBase() = default;

            void reset(jobject* p) { ptr = p; }

        public:
            explicit operator bool() const { return ptr; }

            friend bool operator==(const ObjectBase& a, const ObjectBase& b) { return a.ptr == b.ptr; }
            friend bool operator!=(const ObjectBase& a, const ObjectBase& b) { return !( a == b ); }

            template < class OtherTag >
            bool IsInstanceOf(JNIEnv& env, const Class<OtherTag>& clazz) const
               {
                return jni::IsInstanceOf(env, ptr, clazz);
               }
       };

    template < class Tag, class Enable = void >
    struct SuperObject;

    template < class Tag >
    struct SuperObject<Tag, typename Tag::SuperTag>
       {
        using Type = Object<typename Tag::SuperTag>;
       };

    template < class Tag >
    struct SuperObject<Tag>
       {
        using Type = Object<ObjectTag>;
       };

    template <>
    struct SuperObject<ObjectTag>
        {
         using Type = ObjectBase;
        };

    template < class TheTag = ObjectTag >
    class Object : public SuperObject<TheTag>::Type
       {
        public:
            using TagType = TheTag;
            using UntaggedType = std::conditional_t< std::is_same< TheTag, StringTag >::value, jstring, jobject >;

        protected:
            explicit Object(std::nullptr_t = nullptr)
               {}

            explicit Object(UntaggedType* p)
               : SuperObject<TagType>::Type(p)
               {}

            Object(const Object&) = delete;
            Object& operator=(const Object&) = delete;

        public:
            UntaggedType* Get() const { return reinterpret_cast<UntaggedType*>(this->ptr); }

            operator UntaggedType*() const { return Get(); }
            UntaggedType& operator*() const { return *Get(); }

            template < class T >
            auto Get(JNIEnv& env, const Field<TagType, T>& field) const
               -> std::enable_if_t< IsPrimitive<T>::value, T >
               {
                return GetField<T>(env, this->ptr, field);
               }

            template < class T >
            auto Get(JNIEnv& env, const Field<TagType, T>& field) const
               -> std::enable_if_t< !IsPrimitive<T>::value, Local<T> >
               {
                return Local<T>(env, reinterpret_cast<typename T::UntaggedType*>(GetField<jobject*>(env, this->ptr, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const Field<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetField<T>(env, this->ptr, field, value);
               }

            template < class Expected, class Actual >
            auto Set(JNIEnv& env, const Field<TagType, Expected>& field, const Actual& value) const
               -> std::enable_if_t< !IsPrimitive<Expected>::value
                                 && std::is_convertible<const Actual&, const Expected&>::value >
               {
                SetField<jobject*>(env, this->ptr, field, value.Get());
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, R >
               {
                return CallMethod<R>(env, this->ptr, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value
                                 && !std::is_void<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, Local<R> >
               {
                return Local<R>(env, reinterpret_cast<typename R::UntaggedType*>(CallMethod<jobject*>(env, this->ptr, method, Untag(args)...)));
               }

            template < class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const Method<TagType, void (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value >
               {
                CallMethod<void>(env, this->ptr, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, R >
               {
                return CallNonvirtualMethod<R>(env, this->ptr, clazz, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value
                                 && !std::is_void<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, Local<R> >
               {
                return Local<R>(env, reinterpret_cast<typename R::UntaggedType*>(CallNonvirtualMethod<jobject*>(env, this->ptr, clazz, method, Untag(args)...)));
               }

            template < class... ExpectedArgs, class... ActualArgs >
            auto CallNonvirtual(JNIEnv& env, const Class<TagType>& clazz, const Method<TagType, void (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value >
               {
                CallNonvirtualMethod<void>(env, this->ptr, clazz, method, Untag(args)...);
               }

            template < template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
            Global<Object<TagType>, Deleter> NewGlobalRef(JNIEnv& env) const
               {
                return Global<Object<TagType>, Deleter>(env, reinterpret_cast<typename Object<TagType>::UntaggedType*>(jni::NewGlobalRef(env, this->ptr).release()));
               }

            template < template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
            Weak<Object<TagType>, Deleter> NewWeakGlobalRef(JNIEnv& env) const
               {
                return Weak<Object<TagType>, Deleter>(env, reinterpret_cast<typename Object<TagType>::UntaggedType*>(jni::NewWeakGlobalRef(env, this->ptr).release()));
               }

            Local<Object<TagType>> NewLocalRef(JNIEnv& env) const
               {
                return Local<Object<TagType>>(env, reinterpret_cast<typename Object<TagType>::UntaggedType*>(jni::NewLocalRef(env, this->ptr).release()));
               }
       };

    template < class OutTagType, class T >
    Local<Object<OutTagType>> Cast(JNIEnv& env, const Class<OutTagType>& clazz, const T& object)
       {
        if (!object.IsInstanceOf(env, clazz))
           {
            ThrowNew(env, FindClass(env, "java/lang/ClassCastException"));
           }
        return Local<Object<OutTagType>>(env, reinterpret_cast<typename Object<OutTagType>::UntaggedType*>(object.NewLocalRef(env).release()));
       }
   }
