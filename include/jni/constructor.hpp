#pragma once

#include <jni/method.hpp>

namespace jni
   {

    template < class Tag, class... Args >
    using Constructor = TypedConstructor< TypeFromTag<Tag>, Args... >;

    template < class TagType, class... Args >
    class TypedConstructor : public TypedMethod<TagType, void (Args...)>
       {
        public:
            TypedConstructor(JNIEnv& env, const TypedClass<TagType>& clazz)
               : TypedMethod<TagType, void (Args...)>(env, clazz, "<init>")
               {}
       };
   }
