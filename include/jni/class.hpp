#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class > class StaticField;
    template < class TheTag, class > class StaticMethod;

    template < class TheTag >
    class Class
       {
        private:
            UniqueGlobalRef<jclass> reference;
            jclass* clazz = nullptr;

        public:
            using TagType = TheTag;

            explicit Class(jclass* c)
               : clazz(c)
               {}

            explicit Class(JNIEnv& env)
               : reference(NewGlobalRef(env, FindClass(env, TagType::Name()))),
                 clazz(reference.get())
               {}

            explicit operator bool() const { return clazz; }
            jclass& operator*() const { return *clazz; }
            jclass* Get() const { return clazz; }

            template < class T >
            T Get(JNIEnv& env, const StaticField<TagType, T>& field) const
               {
                return Tag<T>(GetStaticField<UntaggedType<T>>(env, *clazz, *field));
               }

            template < class T >
            void Set(JNIEnv& env, const StaticField<TagType, T>& field, const T& value) const
               {
                SetStaticField(env, *clazz, *field, Untag(value));
               }

            template < class R, class... Args >
            R Call(JNIEnv& env, const StaticMethod<TagType, R (Args...)>& method, const Args&... args) const
               {
                return Tag<R>(CallStaticMethod<UntaggedType<R>>(env, *clazz, *method, Untag(args)...));
               }

            template < class... Args >
            void Call(JNIEnv& env, const StaticMethod<TagType, void (Args...)>& method, const Args&... args) const
              {
               CallStaticMethod<void>(env, *clazz, *method, Untag(args)...);
              }
       };
   }
