#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TagType, class T >
    class TypedStaticField;

    template < class Tag, class T >
    using StaticField = TypedStaticField< TypeFromTag<Tag>, T >;

    template <char... Chars, class T>
    class TypedStaticField< Type<Chars...>, T >
       {
        private:
            jfieldID& field;

        public:
            using TagType = Type<Chars...>;

            TypedStaticField(JNIEnv& env, const TypedClass<TagType>& clazz, const char* name)
              : field(GetStaticFieldID(env, clazz, name, TypeSignature<T>()()))
               {}

            operator jfieldID&() const { return field; }
       };
   }
