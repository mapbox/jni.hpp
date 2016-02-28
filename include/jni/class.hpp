#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>

namespace jni
   {
    template < class TheTag >
    class Class
       {
        private:
            UniqueGlobalRef<jclass> reference;
            jclass* clazz = nullptr;

        public:
            using TagType = TheTag;

            explicit Class(jclass* c)
               : clazz(c)
               {}

            explicit Class(JNIEnv& env)
               : reference(NewGlobalRef(env, FindClass(env, TagType::Name()))),
                 clazz(reference.get())
               {}

            explicit operator bool() const { return clazz; }
            jclass& operator*() const { return *clazz; }
            jclass* Get() const { return clazz; }
       };
   }
