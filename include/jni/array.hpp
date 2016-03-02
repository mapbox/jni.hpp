#pragma once

#include <jni/functions.hpp>
#include <jni/object.hpp>
#include <jni/tagging.hpp>
#include <jni/make.hpp>

namespace jni
   {
    template < class E, class Enable = void > class Array;

    template < class E >
    class Array< E, std::enable_if_t<IsPrimitive<E>::value> >
       {
        public:
            using ElementType = E;
            using UntaggedType = jarray<E>;

        private:
            UniqueGlobalRef<UntaggedType> reference;
            UntaggedType* array = nullptr;

        public:
            explicit Array(UntaggedType* a)
               : array(a)
               {}

            explicit Array(UniqueGlobalRef<UntaggedType>&& r)
               : reference(std::move(r)),
                 array(reference.get())
               {}

            explicit operator bool() const { return array; }
            UntaggedType& operator*() const { return *array; }
            UntaggedType* Get() const { return array; }

            jsize Length(JNIEnv& env) const
               {
                return GetArrayLength(env, SafeDereference(env, array));
               }

            ElementType Get(JNIEnv& env, jsize index) const
               {
                ElementType e;
                GetArrayRegion(env, SafeDereference(env, array), index, 1, &e);
                return e;
               }

            void Set(JNIEnv& env, jsize index, const ElementType& value)
               {
                SetArrayRegion(env, SafeDereference(env, array), index, 1, &value);
               }

            template < class Array >
            void GetRegion(JNIEnv& env, jsize start, Array& buf) const
               {
                GetArrayRegion(env, SafeDereference(env, array), start, buf);
               }

            template < class Array >
            void SetRegion(JNIEnv& env, jsize start, const Array& buf)
               {
                SetArrayRegion(env, SafeDereference(env, array), start, buf);
               }
       };

    template < class TheTag >
    class Array< Object<TheTag> >
       {
        public:
            using TagType = TheTag;
            using ElementType = Object<TagType>;
            using UntaggedType = jarray<jobject>;
            using UntaggedElementType = typename ElementType::UntaggedObjectType;

        private:
            UniqueGlobalRef<UntaggedType> reference;
            UntaggedType* array = nullptr;

        public:
            explicit Array(UntaggedType* a)
               : array(a)
               {}

            explicit Array(UniqueGlobalRef<UntaggedType>&& r)
               : reference(std::move(r)),
                 array(reference.get())
               {}

            explicit operator bool() const { return array; }
            UntaggedType& operator*() const { return *array; }
            UntaggedType* Get() const { return array; }

            jsize Length(JNIEnv& env) const
               {
                return GetArrayLength(env, SafeDereference(env, array));
               }

            ElementType Get(JNIEnv& env, jsize index) const
               {
                return ElementType(
                    reinterpret_cast<UntaggedElementType*>(
                        GetObjectArrayElement(env, SafeDereference(env, array), index)));
               }

            void Set(JNIEnv& env, jsize index, const ElementType& value)
               {
                SetObjectArrayElement(env, SafeDereference(env, array), index, Untag(value));
               }
       };


    template < class T >
    std::vector<T> MakeAnything(ThingToMake<std::vector<T>>, JNIEnv& env, const Array<T>& array)
       {
        NullCheck(env, array.Get());
        std::vector<T> result(GetArrayLength(env, *array));
        GetArrayRegion(env, *array, 0, result);
        return result;
       }

    template < class T >
    Array<T> MakeAnything(ThingToMake<Array<T>>, JNIEnv& env, const std::vector<T>& array)
       {
        Array<T> result(&NewArray<T>(env, array.size()));
        SetArrayRegion(env, *result, 0, array);
        return result;
       }
   }
