#pragma once

#include <jni/functions.hpp>
#include <jni/tagging.hpp>

#include <cstddef>

namespace jni
   {
    struct ObjectTag { static constexpr auto Name() { return "java/lang/Object"; } };

    template < class TagType >
    struct UntaggedObjectType { using Type = jobject; };

    template < class TheTag = ObjectTag >
    class Object
       {
        public:
            using TagType = TheTag;
            using UntaggedType = typename UntaggedObjectType<TagType>::Type;

        private:
            UniqueGlobalRef<UntaggedType> reference;
            UntaggedType* obj = nullptr;

        public:
            explicit Object(std::nullptr_t = nullptr)
               {}

            explicit Object(UntaggedType* o)
               : obj(o)
               {}

            explicit Object(UniqueGlobalRef<UntaggedType>&& r)
               : reference(std::move(r)),
                 obj(reference.get())
               {}

            explicit operator bool() const { return obj; }
            UntaggedType& operator*() const { return *obj; }
            UntaggedType* Get() const { return obj; }
       };
   }
