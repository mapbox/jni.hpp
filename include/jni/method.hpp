#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class >
    class Method;

    template < class TheTag, class R, class... Args >
    class Method< TheTag, R (Args...) >
       {
        public:
            using TagType = TheTag;

        private:
            using TaggedResultType = R;
            using UntaggedResultType = UntaggedType<R>;

            jmethodID& method;

        public:
            Method(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : method(GetMethodID(env, *clazz, name, TypeSignature<R (Args...)>()()))
               {}

            R operator()(JNIEnv& env, const Object<TagType>& instance, const Args&... args) const
               {
                return Tag<R>(CallMethod<UntaggedResultType>(env, instance.Get(), method, Untag(args)...));
               }

            R operator()(JNIEnv& env, const Object<TagType>& instance, const Class<TagType>& clazz, const Args&... args) const
               {
                return Tag<R>(CallNonvirtualMethod<UntaggedResultType>(env, instance.Get(), clazz.Get(), method, Untag(args)...));
               }
       };

    template < class TheTag, class... Args >
    class Method< TheTag, void (Args...) >
       {
        public:
            using TagType = TheTag;

        private:
            jmethodID& method;

        public:
            Method(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : method(GetMethodID(env, *clazz, name, TypeSignature<void (Args...)>()()))
               {}

            void operator()(JNIEnv& env, const Object<TagType>& instance, const Args&... args) const
              {
               CallMethod<void>(env, instance.Get(), method, Untag(args)...);
              }

            void operator()(JNIEnv& env, const Object<TagType>& instance, const Class<TagType>& clazz, const Args&... args) const
              {
               CallNonvirtualMethod<void>(env, instance.Get(), clazz.Get(), method, Untag(args)...);
              }
       };
   }
