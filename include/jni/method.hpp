#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TagType, class >
    class TypedMethod;

    template < class Tag, class R, class... Args >
    using Method = TypedMethod< TypeFromTag<Tag>, R (Args...) >;

    template < char... Chars, class R, class... Args >
    class TypedMethod< Type<Chars...>, R (Args...) >
       {
        private:
            jmethodID& method;

        public:
            using TagType = Type<Chars...>;

            TypedMethod(JNIEnv& env, const TypedClass<TagType>& clazz, const char* name)
              : method(GetMethodID(env, clazz, name, TypeSignature<R (Args...)>()()))
               {}

            operator jmethodID&() const { return method; }
       };
   }
