#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag > class Object;
    template < class TheTag, class... > class Constructor;
    template < class TheTag, class > class StaticField;
    template < class TheTag, class > class StaticMethod;

    template < class TheTag >
    class Class
       {
        private:
            UniqueGlobalRef<jclass> reference;
            jclass& clazz;

        public:
            using TagType = TheTag;

            explicit Class(jclass& c)
               : clazz(c)
               {}

            explicit Class(UniqueGlobalRef<jclass>&& r)
               : reference(std::move(r)),
                 clazz(*reference)
               {}

            operator jclass&() const { return clazz; }
            jclass* Get() const { return &clazz; }

            template < class... Args >
            Object<TagType> New(JNIEnv& env, const Constructor<TagType, Args...>& method, const Args&... args) const
               {
                return Object<TagType>(&NewObject(env, clazz, method, Untag(args)...));
               }

            template < class T >
            T Get(JNIEnv& env, const StaticField<TagType, T>& field) const
               {
                return Tag<T>(GetStaticField<UntaggedType<T>>(env, clazz, field));
               }

            template < class T >
            void Set(JNIEnv& env, const StaticField<TagType, T>& field, const T& value) const
               {
                SetStaticField(env, clazz, field, Untag(value));
               }

            template < class R, class... Args >
            R Call(JNIEnv& env, const StaticMethod<TagType, R (Args...)>& method, const Args&... args) const
               {
                return Tag<R>(CallStaticMethod<UntaggedType<R>>(env, clazz, method, Untag(args)...));
               }

            template < class... Args >
            void Call(JNIEnv& env, const StaticMethod<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallStaticMethod<void>(env, clazz, method, Untag(args)...);
               }

            Class NewGlobalRef(JNIEnv& env) const
               {
                return Class(jni::NewGlobalRef(env, &clazz));
               }

            Class Release()
               {
                return Class(*reference.release());
               }

            static Class Find(JNIEnv& env)
               {
                return Class(FindClass(env, TagType::Name()));
               }
       };
   }
