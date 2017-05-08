#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TagType, class >
    class TypedStaticMethod;

    template < class Tag, class R, class... Args >
    using StaticMethod = TypedStaticMethod< TypeFromTag<Tag>, R (Args...) >;

    template <char... Chars, class R, class... Args>
    class TypedStaticMethod< Type<Chars...>, R (Args...) >
       {
        private:
            jmethodID& method;

        public:
            using TagType = Type<Chars...>;

            TypedStaticMethod(JNIEnv& env, const TypedClass<TagType>& clazz, const char* name)
              : method(GetStaticMethodID(env, clazz, name, TypeSignature<R (Args...)>()()))
               {}

            operator jmethodID&() const { return method; }
       };
   }
