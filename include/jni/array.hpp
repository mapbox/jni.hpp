#pragma once

#include <jni/functions.hpp>
#include <jni/object.hpp>
#include <jni/tagging.hpp>
#include <jni/make.hpp>
#include <jni/unique_pointerlike.hpp>

namespace jni
   {
    template < class E, class Enable = void >
    class Array;

    template < class E >
    class Array< E, std::enable_if_t<IsPrimitive<E>::value> >
       {
        public:
            using ElementType = E;
            using UntaggedType = jarray<E>;

        private:
            UntaggedType* array = nullptr;

            template < class T, class D > friend class UniquePointerlike;

            void reset(UntaggedType* a) { array = a; }

        public:
            explicit Array(std::nullptr_t = nullptr)
               {}

            explicit Array(UntaggedType* a)
               : array(a)
               {}

            explicit Array(UntaggedType& a)
               : array(&a)
               {}

            Array(const Array& a)
               : array(a.array)
               {}

            // Not reassignable; it would break UniquePointerlike's abstraction.
            Array& operator=(const Array&) = delete;

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

            Global<Array<E>> NewGlobalRef(JNIEnv& env) const
               {
                return SeizeGlobal(env, Array(jni::NewGlobalRef(env, array).release()));
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

            template < class T, class D > friend class UniquePointerlike;

            void reset(UntaggedType* a) { array = a; }

        public:
            explicit Array(std::nullptr_t = nullptr)
               {}

            explicit Array(UntaggedType* a)
               : array(a)
               {}

            explicit Array(UntaggedType& a)
               : array(&a)
               {}

            Array(const Array& a)
               : array(a.array)
               {}

            // Not reassignable; it would break UniquePointerlike's abstraction.
            Array& operator=(const Array&) = delete;

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
                SetObjectArrayElement(env, SafeDereference(env, array), index, RemoveTag(value));
               }

            static Array<Object<TheTag>> New(JNIEnv& env, jsize length, const Class<TheTag>& clazz, const Object<TheTag>& initialElement = Object<TheTag>())
               {
                return Array<Object<TheTag>>(&NewObjectArray(env, length, clazz, initialElement.Get()));
               }

            Global<Array<Object<TheTag>>> NewGlobalRef(JNIEnv& env) const
               {
                return SeizeGlobal(env, Array(jni::NewGlobalRef(env, array).release()));
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

    inline
    std::string MakeAnything(ThingToMake<std::string>, JNIEnv& env, const Array<jbyte>& array)
       {
        NullCheck(env, array.Get());
        std::string result;
        result.resize(GetArrayLength(env, *array));
        GetArrayRegion(env, *array, 0, result.size(), reinterpret_cast<jbyte*>(&result[0]));
        return result;
       }

    inline
    Array<jbyte> MakeAnything(ThingToMake<Array<jbyte>>, JNIEnv& env, const std::string& string)
       {
        Array<jbyte> result(&NewArray<jbyte>(env, string.size()));
        SetArrayRegion(env, *result, 0, string.size(), reinterpret_cast<const jbyte*>(&string[0]));
        return result;
       }
   }
