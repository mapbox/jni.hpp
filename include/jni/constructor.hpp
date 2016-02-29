#pragma once

#include <jni/functions.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include <jni/type_signature.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag, class... Args >
    class Constructor
       {
        public:
            using TagType = TheTag;

        private:
            jmethodID& method;

        public:
            Constructor(JNIEnv& env, const Class<TagType>& clazz)
              : method(GetMethodID(env, *clazz, "<init>", TypeSignature<void (Args...)>()()))
               {}

            jmethodID& operator*() const { return method; }
       };
   }
