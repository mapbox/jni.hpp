#pragma once

#include <jni/functions.hpp>
#include <jni/object.hpp>
#include <jni/tagging.hpp>
#include <jni/make.hpp>

namespace jni
   {
    template < class E, class Enable = void > class Array;

    template < class E >
    class Array< E, typename std::enable_if<IsPrimitive<E>::value>::type >
       {
        public:
            using ElementType = E;
            using UntaggedType = jarray<E>;

        private:
            UniqueGlobalRef<UntaggedType> reference;
            UntaggedType* array = nullptr;

        public:
            Array(UntaggedType* a)
               : array(a)
               {}

            Array(UniqueGlobalRef<UntaggedType>&& r)
               : reference(std::move(r)),
                 array(reference.get())
               {}

            explicit operator bool() const { return array; }
            UntaggedType& operator*() const { return *array; }
            UntaggedType* Get() const { return array; }

            jsize Length(JNIEnv& env) const
               {
                return GetArrayLength(env, array);
               }

            ElementType Get(JNIEnv& env, jsize index) const
               {
                ElementType e;
                GetArrayRegion<ElementType>(env, array, index, 1, &e);
                return e;
               }
       };

    template < class TheTag >
    class Array< Object<TheTag> >
       {
        public:
            using TagType = TheTag;
            using ElementType = Object<TagType>;
            using UntaggedType = jarray<jobject>;
            using UntaggedElementType = typename ElementType::UntaggedType;

        private:
            UniqueGlobalRef<UntaggedType> reference;
            UntaggedType* array = nullptr;

        public:
            Array(UntaggedType* a)
               : array(a)
               {}

            Array(UniqueGlobalRef<UntaggedType>&& r)
               : reference(std::move(r)),
                 array(reference.get())
               {}

            explicit operator bool() const { return array; }
            UntaggedType& operator*() const { return *array; }
            UntaggedType* Get() const { return array; }

            jsize Length(JNIEnv& env) const
               {
                return GetArrayLength(env, array);
               }

            ElementType Get(JNIEnv& env, jsize index) const
               {
                return ElementType(
                    reinterpret_cast<UntaggedElementType*>(GetObjectArrayElement(env, array, index)));
               }
       };


    template < class T >
    std::vector<T> MakeAnything(ThingToMake<std::vector<T>>, JNIEnv& env, const Array<T>& array)
       {
        std::vector<T> result(array.Length(env));
        jni::GetArrayRegion<T>(env, array.Get(), 0, result);
        return result;
       }

    template < class T >
    Array<T> MakeAnything(ThingToMake<Array<T>>, JNIEnv& env, const std::vector<T>& array)
       {
        Array<T> result(&NewArray<T>(env, array.size()));
        jni::SetArrayRegion<T>(env, result.Get(), 0, array);
        return result;
       }
   }
