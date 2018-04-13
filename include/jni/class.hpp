#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>
#include <jni/unique_pointerlike.hpp>

namespace jni
   {
    template < class TheTag > class Object;
    template < class TheTag, class... > class Constructor;
    template < class TheTag, class > class Field;
    template < class TheTag, class > class StaticField;
    template < class TheTag, class > class Method;
    template < class TheTag, class > class StaticMethod;

    template < class TheTag >
    class Class
       {
        private:
            jclass* clazz = nullptr;

            template < class T, class D > friend class UniquePointerlike;

            void reset(jclass* c) { clazz = c; }

        public:
            using TagType = TheTag;

            explicit Class(std::nullptr_t = nullptr)
               {}

            explicit Class(jclass& c)
               : clazz(&c)
               {}

            Class(const Class& c)
               : clazz(c.clazz)
               {}

            // Not reassignable; it would break UniquePointerlike's abstraction.
            Class& operator=(const Class&) = delete;

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
               -> std::enable_if_t< !IsPrimitive<R>::value && !std::is_void<R>::value, R >
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

            static const Class& Singleton(JNIEnv& env)
               {
                // The global references created here is purposefully leaked. Due to the design
                // of Java/JNI, there is virtually no way to reliably release them. See
                // http://bleaklow.com/2006/02/18/jni_onunload_mostly_useless.html for details.
                static Class singleton = Find(env).NewGlobalRef(env).release();
                return singleton;
               }

            template < class... Args >
            Constructor<TagType, Args...> GetConstructor(JNIEnv& env) const
               {
                return Constructor<TagType, Args...>(env, *this);
               }

            template < class T >
            Field<TagType, T> GetField(JNIEnv& env, const char* name) const
               {
                return Field<TagType, T>(env, *this, name);
               }

            template < class T >
            StaticField<TagType, T> GetStaticField(JNIEnv& env, const char* name) const
               {
                return StaticField<TagType, T>(env, *this, name);
               }

            template < class T >
            Method<TagType, T> GetMethod(JNIEnv& env, const char* name) const
               {
                return Method<TagType, T>(env, *this, name);
               }

            template < class T >
            StaticMethod<TagType, T> GetStaticMethod(JNIEnv& env, const char* name) const
               {
                return StaticMethod<TagType, T>(env, *this, name);
               }

            Global<Class<TagType>> NewGlobalRef(JNIEnv& env) const
               {
                return SeizeGlobal(env, Class(*jni::NewGlobalRef(env, clazz).release()));
               }
       };
   }
