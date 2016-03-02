#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>
#include <jni/pointer_to_value.hpp>

namespace jni
   {
    template < class TheTag > class Object;
    template < class TheTag, class... > class Constructor;
    template < class TheTag, class > class Field;
    template < class TheTag, class > class StaticField;
    template < class TheTag, class > class Method;
    template < class TheTag, class > class StaticMethod;

    template < class TheTag >
    class Class;

    template < class TagType >
    class ClassDeleter;

    template < class TagType >
    using UniqueClass = std::unique_ptr< const Class<TagType>, ClassDeleter<TagType> >;

    template < class TheTag >
    class Class
       {
        private:
            jclass* clazz = nullptr;

        public:
            using TagType = TheTag;

            explicit Class(std::nullptr_t = nullptr)
               {}

            explicit Class(jclass& c)
               : clazz(&c)
               {}

            explicit operator bool() const { return clazz; }

            operator jclass&() const { return *clazz; }
            jclass& operator*() const { return *clazz; }
            jclass* Get() const { return clazz; }

            friend bool operator==( const Class& a, const Class& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const Class& a, const Class& b )  { return !( a == b ); }

            template < class... Args >
            Object<TagType> New(JNIEnv& env, const Constructor<TagType, Args...>& method, const Args&... args) const
               {
                return Object<TagType>(&NewObject(env, *clazz, method, Untag(args)...));
               }

            template < class T >
            auto Get(JNIEnv& env, const StaticField<TagType, T>& field) const
               -> std::enable_if_t< IsPrimitive<T>::value, T >
               {
                return jni::GetStaticField<T>(env, *clazz, field);
               }

            template < class T >
            auto Get(JNIEnv& env, const StaticField<TagType, T>& field) const
               -> std::enable_if_t< !IsPrimitive<T>::value, T >
               {
                return T(reinterpret_cast<UntaggedType<T>>(jni::GetStaticField<jobject*>(env, *clazz, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const StaticField<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetStaticField<T>(env, *clazz, field, value);
               }

            template < class T >
            auto Set(JNIEnv& env, const StaticField<TagType, T>& field, const T& value) const
               -> std::enable_if_t< !IsPrimitive<T>::value >
               {
                SetStaticField<jobject*>(env, *clazz, field, value.Get());
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const StaticMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value, R >
               {
                return CallStaticMethod<R>(env, *clazz, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const StaticMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value, R >
               {
                return R(reinterpret_cast<UntaggedType<R>>(CallStaticMethod<jobject*>(env, *clazz, method, Untag(args)...)));
               }

            template < class... Args >
            void Call(JNIEnv& env, const StaticMethod<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallStaticMethod<void>(env, *clazz, method, Untag(args)...);
               }

            static Class Find(JNIEnv& env)
               {
                return Class(FindClass(env, TagType::Name()));
               }

            template < class... Args >
            Constructor<TagType, Args...> GetConstructor(JNIEnv& env)
               {
                return Constructor<TagType, Args...>(env, *this);
               }

            template < class T >
            Field<TagType, T> GetField(JNIEnv& env, const char* name)
               {
                return Field<TagType, T>(env, *this, name);
               }

            template < class T >
            StaticField<TagType, T> GetStaticField(JNIEnv& env, const char* name)
               {
                return StaticField<TagType, T>(env, *this, name);
               }

            template < class T >
            Method<TagType, T> GetMethod(JNIEnv& env, const char* name)
               {
                return Method<TagType, T>(env, *this, name);
               }

            template < class T >
            StaticMethod<TagType, T> GetStaticMethod(JNIEnv& env, const char* name)
               {
                return StaticMethod<TagType, T>(env, *this, name);
               }

            UniqueClass<TagType> NewGlobalRef(JNIEnv& env) const
               {
                return Seize(env, Class(*jni::NewGlobalRef(env, clazz).release()));
               }
       };

    template < class TagType >
    class ClassDeleter
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< Class<TagType> >;

            ClassDeleter() = default;
            ClassDeleter(JNIEnv& e) : env(&e) {}

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
    UniqueClass<TagType> Seize(JNIEnv& env, Class<TagType>&& clazz)
       {
        return UniqueClass<TagType>(PointerToValue<Class<TagType>>(std::move(clazz)), ClassDeleter<TagType>(env));
       };
   }
