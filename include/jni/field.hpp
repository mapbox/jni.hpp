#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class T >
    class Field
       {
        public:
            using TagType = TheTag;
            using Type = T;

        private:
            using UntaggedType = UntaggedType<T>;

            jfieldID& field;

        public:
            Field(JNIEnv& env, const Class<TagType>& clazz, const char* name)
              : field(GetFieldID(env, *clazz, name, TypeSignature<T>()()))
               {}

            T Get(JNIEnv& env, const Object<TagType>& obj) const
               {
                return Tag<T>(GetField<UntaggedType>(env, obj.Get(), field));
               }

            void Set(JNIEnv& env, const Object<TagType>& obj, const T& value)
               {
                SetField(env, obj.Get(), field, Untag(value));
               }
       };
   }
