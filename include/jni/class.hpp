#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>
#include <jni/advanced_ownership.hpp>

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
            using TagType = TheTag;

            jclass* clazz = nullptr;

        protected:
            explicit Class(std::nullptr_t = nullptr)
               {}

            explicit Class(jclass* c)
               : clazz(c)
               {}

            Class(const Class& c)
               : clazz(c.clazz)
               {}

            ~Class() = default;

            Class& operator=(const Class&) = delete;
            void reset(jclass* c) { clazz = c; }

        public:
            explicit operator bool() const { return clazz; }

            operator jclass&() const { return *clazz; }
            jclass& operator*() const { return *clazz; }
            jclass* Get() const { return clazz; }

            friend bool operator==( const Class& a, const Class& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const Class& a, const Class& b )  { return !( a == b ); }

            template < class... ExpectedArgs, class... ActualArgs >
            auto New(JNIEnv& env, const Constructor<TagType, ExpectedArgs...>& method, const ActualArgs&... args) const
               -> std::enable_if_t< Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, Local<Object<TagType>> >
               {
                return Local<Object<TagType>>(env, &NewObject(env, *clazz, method, Untag(args)...));
               }

            template < class T >
            auto Get(JNIEnv& env, const StaticField<TagType, T>& field) const
               -> std::enable_if_t< IsPrimitive<T>::value, T >
               {
                return jni::GetStaticField<T>(env, *clazz, field);
               }

            template < class T >
            auto Get(JNIEnv& env, const StaticField<TagType, T>& field) const
               -> std::enable_if_t< !IsPrimitive<T>::value, Local<T> >
               {
                return Local<T>(env, reinterpret_cast<UntaggedType<T>>(jni::GetStaticField<jobject*>(env, *clazz, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const StaticField<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetStaticField<T>(env, *clazz, field, value);
               }

            template < class Expected, class Actual >
            auto Set(JNIEnv& env, const StaticField<TagType, Expected>& field, const Actual& value) const
               -> std::enable_if_t< !IsPrimitive<Expected>::value
                                 && std::is_convertible<const Actual&, const Expected&>::value >
               {
                SetStaticField<jobject*>(env, *clazz, field, value.Get());
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const StaticMethod<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, R >
               {
                return CallStaticMethod<R>(env, *clazz, method, Untag(args)...);
               }

            template < class R, class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const StaticMethod<TagType, R (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value
                                 && !std::is_void<R>::value
                                 && Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value, Local<R> >
               {
                return Local<R>(env, reinterpret_cast<UntaggedType<R>>(CallStaticMethod<jobject*>(env, *clazz, method, Untag(args)...)));
               }

            template < class... ExpectedArgs, class... ActualArgs >
            auto Call(JNIEnv& env, const StaticMethod<TagType, void (ExpectedArgs...)>& method, const ActualArgs&... args) const
               -> std::enable_if_t< Conjunction<std::is_convertible<const ActualArgs&, const ExpectedArgs&>...>::value >
               {
                CallStaticMethod<void>(env, *clazz, method, Untag(args)...);
               }

            static Local<Class> Find(JNIEnv& env)
               {
                return Local<Class>(env, &FindClass(env, TagType::Name()));
               }

            static const Class& Singleton(JNIEnv& env)
               {
                static Global<Class, EnvIgnoringDeleter> singleton = Find(env).template NewGlobalRef<EnvIgnoringDeleter>(env);
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

            template < template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
            Global<Class<TagType>, Deleter> NewGlobalRef(JNIEnv& env) const
               {
                return Global<Class<TagType>, Deleter>(env, jni::NewGlobalRef(env, clazz).release());
               }
       };
   }
