#pragma once

#include <jni/functions.hpp>
#include <jni/object.hpp>
#include <jni/tagging.hpp>
#include <jni/make.hpp>
#include <jni/pointer_to_value.hpp>

namespace jni
   {
    template < class E, class Enable = void >
    class Array;

    template < class TagType >
    class ArrayDeleter;

    template < class E >
    using UniqueArray = std::unique_ptr< const Array<E>, ArrayDeleter<E> >;

    template < class E >
    class Array< E, std::enable_if_t<IsPrimitive<E>::value> >
       {
        public:
            using ElementType = E;
            using UntaggedType = jarray<E>;

        private:
            UntaggedType* array = nullptr;

        public:
            explicit Array(UntaggedType* a)
               : array(a)
               {}

            explicit operator bool() const { return array; }

            operator UntaggedType*() const { return array; }
            UntaggedType& operator*() const { return *array; }
            UntaggedType* Get() const { return array; }

            friend bool operator==( const Array& a, const Array& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const Array& a, const Array& b )  { return !( a == b ); }

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

            static Array<E> New(JNIEnv& env, jsize length)
               {
                return Array<E>(&NewArray<E>(env, length));
               }

            UniqueArray<E> NewGlobalRef(JNIEnv& env) const
               {
                return Seize(env, Array(jni::NewGlobalRef(env, array).release()));
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
            UntaggedType* array = nullptr;

        public:
            explicit Array(UntaggedType* a)
               : array(a)
               {}

            explicit operator bool() const { return array; }

            operator UntaggedType*() const { return array; }
            UntaggedType& operator*() const { return *array; }
            UntaggedType* Get() const { return array; }

            friend bool operator==( const Array& a, const Array& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const Array& a, const Array& b )  { return !( a == b ); }

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

            static Array<Object<TheTag>> New(JNIEnv& env, jsize length, const Class<TheTag>& clazz, const Object<TheTag>& initialElement = Object<TheTag>())
               {
                return Array<Object<TheTag>>(&NewObjectArray(env, length, clazz, initialElement.Get()));
               }

            UniqueArray<Object<TheTag>> NewGlobalRef(JNIEnv& env) const
               {
                return Seize(env, Array(jni::NewGlobalRef(env, array).release()));
               }
      };

    template < class E >
    class ArrayDeleter
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< Array<E> >;

            ArrayDeleter() = default;
            ArrayDeleter(JNIEnv& e) : env(&e) {}

            void operator()(pointer p) const
               {
                if (p)
                   {
                    assert(env);
                    env->DeleteGlobalRef(Unwrap(p->Get()));
                   }
               }
       };

    template < class E >
    UniqueArray<E> Seize(JNIEnv& env, Array<E>&& array)
       {
        return UniqueArray<E>(PointerToValue<Array<E>>(std::move(array)), ArrayDeleter<E>(env));
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
