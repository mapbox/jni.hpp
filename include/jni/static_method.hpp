#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class >
    class StaticMethod;

    template < class TheTag, class R, class... Args >
    class StaticMethod< TheTag, R (Args...) >
       {
        public:
            using TagType = TheTag;

        private:
            using TaggedResultType = R;
            using UntaggedResultType = UntaggedType<R>;

            jmethodID& method;

        public:
            StaticMethod(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : method(GetStaticMethodID(env, *clazz, name, TypeSignature<R (Args...)>()()))
               {}

            R operator()(JNIEnv& env, const Class<TagType>& clazz, const Args&... args) const
              {
               return Tag<R>(CallStaticMethod<UntaggedResultType>(env, *clazz, method, Untag(args)...));
              }
       };

    template < class TheTag, class... Args >
    class StaticMethod< TheTag, void (Args...) >
       {
        public:
            using TagType = TheTag;

        private:
            jmethodID& method;

        public:
            StaticMethod(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : method(GetStaticMethodID(env, *clazz, name, TypeSignature<void (Args...)>()()))
               {}

            void operator()(JNIEnv& env, const Class<TagType>& clazz, const Args&... args) const
              {
               CallStaticMethod<void>(env, *clazz, method, Untag(args)...);
              }
       };
   }
