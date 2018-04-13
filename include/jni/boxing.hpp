#pragma once

#include <jni/class.hpp>

namespace jni
   {
    template < class > struct Boxer;
    template < class > struct Unboxer;

    template < class Unboxed >
    auto Box(JNIEnv& env, Unboxed&& unboxed)
       {
        return Boxer<typename std::decay<Unboxed>::type>().Box(env, std::forward<Unboxed>(unboxed));
       }

    template < class Tag >
    auto Unbox(JNIEnv& env, const Object<Tag>& boxed)
       {
        return Unboxer<Tag>().Unbox(env, boxed);
       }


    struct BooleanTag
       {
        static constexpr auto Name() { return "java/lang/Boolean"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "booleanValue"; }
       };

    struct ByteTag
       {
        static constexpr auto Name() { return "java/lang/Byte"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "byteValue"; }
       };

    struct CharacterTag
       {
        static constexpr auto Name() { return "java/lang/Character"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "charValue"; }
       };

    struct ShortTag
       {
        static constexpr auto Name() { return "java/lang/Short"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "shortValue"; }
       };

    struct IntegerTag
       {
        static constexpr auto Name() { return "java/lang/Integer"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "intValue"; }
       };

    struct LongTag
       {
        static constexpr auto Name() { return "java/lang/Long"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "longValue"; }
       };

    struct FloatTag
       {
        static constexpr auto Name() { return "java/lang/Float"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "floatValue"; }
       };

    struct DoubleTag
       {
        static constexpr auto Name() { return "java/lang/Double"; }
        static constexpr auto BoxStaticMethodName() { return "valueOf"; }
        static constexpr auto UnboxMethodName() { return "doubleValue"; }
       };


    template < class Tag, class Unboxed >
    struct PrimitiveTypeBoxer
       {
        Object<Tag> Box(JNIEnv& env, Unboxed unboxed) const
           {
            static auto klass = Class<Tag>::Singleton(env);
            static auto box = klass.template GetStaticMethod<Object<Tag> (Unboxed)>(env, Tag::BoxStaticMethodName());
            return klass.Call(env, box, unboxed);
           }
       };

    template <> struct Boxer< jboolean > : PrimitiveTypeBoxer< BooleanTag   , jboolean > {};
    template <> struct Boxer< jbyte    > : PrimitiveTypeBoxer< ByteTag      , jbyte    > {};
    template <> struct Boxer< jchar    > : PrimitiveTypeBoxer< CharacterTag , jchar    > {};
    template <> struct Boxer< jshort   > : PrimitiveTypeBoxer< ShortTag     , jshort   > {};
    template <> struct Boxer< jint     > : PrimitiveTypeBoxer< IntegerTag   , jint     > {};
    template <> struct Boxer< jlong    > : PrimitiveTypeBoxer< LongTag      , jlong    > {};
    template <> struct Boxer< jfloat   > : PrimitiveTypeBoxer< FloatTag     , jfloat   > {};
    template <> struct Boxer< jdouble  > : PrimitiveTypeBoxer< DoubleTag    , jdouble  > {};


    template < class Tag, class Unboxed >
    struct PrimitiveTypeUnboxer
       {
        Unboxed Unbox(JNIEnv& env, const Object<Tag>& boxed) const
           {
            static auto klass = Class<Tag>::Singleton(env);
            static auto unbox = klass.template GetMethod<Unboxed ()>(env, Tag::UnboxMethodName());
            return boxed.Call(env, unbox);
           }
       };

    template <> struct Unboxer< BooleanTag   > : PrimitiveTypeUnboxer< BooleanTag   , jboolean > {};
    template <> struct Unboxer< ByteTag      > : PrimitiveTypeUnboxer< ByteTag      , jbyte    > {};
    template <> struct Unboxer< CharacterTag > : PrimitiveTypeUnboxer< CharacterTag , jchar    > {};
    template <> struct Unboxer< ShortTag     > : PrimitiveTypeUnboxer< ShortTag     , jshort   > {};
    template <> struct Unboxer< IntegerTag   > : PrimitiveTypeUnboxer< IntegerTag   , jint     > {};
    template <> struct Unboxer< LongTag      > : PrimitiveTypeUnboxer< LongTag      , jlong    > {};
    template <> struct Unboxer< FloatTag     > : PrimitiveTypeUnboxer< FloatTag     , jfloat   > {};
    template <> struct Unboxer< DoubleTag    > : PrimitiveTypeUnboxer< DoubleTag    , jdouble  > {};


    template < class Tag >
    struct Boxer<jni::Object<Tag>>
       {
        const Object<Tag>& Box(JNIEnv&, const jni::Object<Tag>& o) const { return o; }
       };

    template < class Tag >
    struct Unboxer
       {
        const Object<Tag>& Unbox(JNIEnv&, const jni::Object<Tag>& o) const { return o; }
       };
   }
