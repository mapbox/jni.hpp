#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class T >
    class StaticField
       {
        public:
            using TagType = TheTag;
            using Type = T;

        private:
            using UntaggedType = UntaggedType<T>;

            jfieldID& field;

        public:
            StaticField(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : field(GetStaticFieldID(env, *clazz, name, TypeSignature<T>()()))
               {}

            T Get(JNIEnv& env, const Class<TagType>& clazz) const
               {
                return Tag<T>(GetStaticField<UntaggedType>(env, *clazz, field));
               }

            void Set(JNIEnv& env, const Class<TagType>& clazz, const T& value)
               {
                SetStaticField(env, *clazz, field, Untag(value));
               }
       };
   }
