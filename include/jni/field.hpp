#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TagType, class T >
    class TypedField;

    template < class Tag, class T >
    using Field = TypedField< TypeFromTag<Tag>, T >;

    template <char... Chars, class T>
    class TypedField< Type<Chars...>, T >
       {
        private:
            jfieldID& field;

        public:
            using TagType = Type<Chars...>;

            TypedField(JNIEnv& env, const TypedClass<TagType>& clazz, const char* name)
              : field(GetFieldID(env, clazz, name, TypeSignature<T>()()))
               {}

            operator jfieldID&() const { return field; }
       };
   }
