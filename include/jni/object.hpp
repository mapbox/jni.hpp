#pragma once

#include <jni/functions.hpp>
#include <jni/type.hpp>
#include <jni/tagging.hpp>
#include <jni/pointer_to_value.hpp>

#include <cstddef>

namespace jni
   {
    template < class TagType > class TypedClass;
    template < class TagType, class > class TypedField;
    template < class TagType, class > class TypedMethod;

    struct ObjectTag { static constexpr auto Name() { return "java/lang/Object"; } };

    template < class TagType >
    struct TypedUntaggedObjectType;

    template <char... Chars>
    struct TypedUntaggedObjectType<Type<Chars...>> { using Type = jobject; };

    template < class TagType = TypeFromTag<ObjectTag> >
    class TypedObject;

    template < class TagType >
    class TypedObjectDeleter;

    template < class TagType = TypeFromTag<ObjectTag> >
    using TypedUniqueObject = std::unique_ptr< const TypedObject<TagType>, TypedObjectDeleter<TagType> >;

    template < class Tag = ObjectTag >
    using UniqueObject = TypedUniqueObject< TypeFromTag<Tag> >;

    template < class TagType >
    class TypedWeakObjectRefDeleter;

    template < class TagType = TypeFromTag<ObjectTag> >
    using TypedUniqueWeakObject = std::unique_ptr< const TypedObject<TagType>, TypedWeakObjectRefDeleter<TagType> >;

    template < class Tag = ObjectTag >
    using UniqueWeakObject = TypedUniqueWeakObject< TypeFromTag<Tag> >;

    template < class TagType >
    class TypedLocalObjectRefDeleter;

    template < class TagType = TypeFromTag<ObjectTag> >
    using TypedUniqueLocalObject = std::unique_ptr< const TypedObject<TagType>, TypedLocalObjectRefDeleter<TagType> >;

    template < class Tag = ObjectTag >
    using UniqueLocalObject = TypedUniqueLocalObject< TypeFromTag<Tag> >;

    template < class Tag = ObjectTag >
    using Object = TypedObject< TypeFromTag<Tag> >;

    template < char... Chars >
    class TypedObject< Type<Chars...> >
       {
        public:
            using TagType = Type<Chars...>;
            using UntaggedObjectType = typename TypedUntaggedObjectType<TagType>::Type;

        private:
            UntaggedObjectType* obj = nullptr;

        public:
            explicit TypedObject(std::nullptr_t = nullptr)
               {}

            explicit TypedObject(UntaggedObjectType* o)
               : obj(o)
               {}

            explicit TypedObject(UntaggedObjectType& o)
               : obj(&o)
               {}

            explicit operator bool() const { return obj; }

            operator UntaggedObjectType*() const { return obj; }
            UntaggedObjectType& operator*() const { return *obj; }
            UntaggedObjectType* Get() const { return obj; }

            friend bool operator==( const TypedObject& a, const TypedObject& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const TypedObject& a, const TypedObject& b )  { return !( a == b ); }

            template < class T >
            auto Get(JNIEnv& env, const TypedField<TagType, T>& field) const
               -> std::enable_if_t< IsPrimitive<T>::value, T >
               {
                return GetField<T>(env, obj, field);
               }

            template < class T >
            auto Get(JNIEnv& env, const TypedField<TagType, T>& field) const
               -> std::enable_if_t< !IsPrimitive<T>::value, T >
               {
                return T(reinterpret_cast<UntaggedType<T>>(GetField<jobject*>(env, obj, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const TypedField<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetField<T>(env, obj, field, value);
               }

            template < class T >
            auto Set(JNIEnv& env, const TypedField<TagType, T>& field, const T& value) const
               -> std::enable_if_t< !IsPrimitive<T>::value >
               {
                SetField<jobject*>(env, obj, field, value.Get());
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const TypedMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value, R >
               {
                return CallMethod<R>(env, obj, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const TypedMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value, R >
               {
                return R(reinterpret_cast<UntaggedType<R>>(CallMethod<jobject*>(env, obj, method, Untag(args)...)));
               }

            template < class... Args >
            void Call(JNIEnv& env, const TypedMethod<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallMethod<void>(env, obj, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto CallNonvirtual(JNIEnv& env, const TypedClass<TagType>& clazz, const TypedMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value, R >
               {
                return CallNonvirtualMethod<R>(env, obj, clazz, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto CallNonvirtual(JNIEnv& env, const TypedClass<TagType>& clazz, const TypedMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value, R >
               {
                return R(reinterpret_cast<UntaggedType<R>>(CallNonvirtualMethod<jobject*>(env, obj, clazz, method, Untag(args)...)));
               }

            template < class... Args >
            void CallNonvirtual(JNIEnv& env, const TypedClass<TagType>& clazz, const TypedMethod<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallNonvirtualMethod<void>(env, obj, clazz, method, Untag(args)...);
               }

            TypedUniqueObject<TagType> NewGlobalRef(JNIEnv& env) const
               {
                return Seize(env, TypedObject(jni::NewGlobalRef(env, obj).release()));
               }

            TypedUniqueWeakObject<TagType> NewWeakGlobalRef(JNIEnv& env) const
               {
                return SeizeWeakRef(env, TypedObject(jni::NewWeakGlobalRef(env, obj).release()));
               }

            TypedUniqueLocalObject<TagType> NewLocalRef(JNIEnv& env) const
               {
                return SeizeLocalRef(env, TypedObject(jni::NewLocalRef(env, obj).release()));
               }

            template < class OtherTag >
            bool IsInstanceOf(JNIEnv& env, const TypedClass<OtherTag>& clazz) const
               {
                return jni::IsInstanceOf(env, obj, clazz);
               }
       };

    template < class Tag = ObjectTag >
    using ObjectDeleter = TypedObjectDeleter< TypeFromTag<Tag> >;

    template < char... Chars >
    class TypedObjectDeleter< Type<Chars...> >
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< TypedObject< Type<Chars...> > >;

            TypedObjectDeleter() = default;
            TypedObjectDeleter(JNIEnv& e) : env(&e) {}

            void operator()(pointer p) const
               {
                if (p)
                   {
                    assert(env);
                    env->DeleteGlobalRef(Unwrap(p->Get()));
                   }
               }
       };

    template < class TagType >
    TypedUniqueObject<TagType> Seize(JNIEnv& env, TypedObject<TagType>&& object)
       {
        return TypedUniqueObject<TagType>(PointerToValue<TypedObject<TagType>>(std::move(object)), TypedObjectDeleter<TagType>(env));
       };

    template < class Tag = ObjectTag >
    using WeakObjectRefDeleter = TypedWeakObjectRefDeleter< TypeFromTag<Tag> >;

    template < char... Chars >
    class TypedWeakObjectRefDeleter< Type<Chars...> >
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< TypedObject< Type<Chars...> > >;

            TypedWeakObjectRefDeleter() = default;
            TypedWeakObjectRefDeleter(JNIEnv& e) : env(&e) {}

            void operator()(pointer p) const
               {
                if (p)
                   {
                    assert(env);
                    env->DeleteWeakGlobalRef(Unwrap(p->Get()));
                   }
               }
       };

    template < class TagType >
    TypedUniqueWeakObject<TagType> SeizeWeakRef(JNIEnv& env, TypedObject<TagType>&& object)
       {
        return TypedUniqueWeakObject<TagType>(PointerToValue<TypedObject<TagType>>(std::move(object)), TypedWeakObjectRefDeleter<TagType>(env));
       };

    template < class Tag = ObjectTag >
    using LocalObjectRefDeleter = TypedLocalObjectRefDeleter< TypeFromTag<Tag> >;

    template < char... Chars >
    class TypedLocalObjectRefDeleter< Type<Chars...> >
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< TypedObject< Type<Chars...> > >;

            TypedLocalObjectRefDeleter() = default;
            TypedLocalObjectRefDeleter(JNIEnv& e) : env(&e) {}

            void operator()(pointer p) const
               {
                if (p)
                   {
                    assert(env);
                    env->DeleteLocalRef(Unwrap(p->Get()));
                   }
               }
       };

    template < class TagType >
    TypedUniqueLocalObject<TagType> SeizeLocalRef(JNIEnv& env, TypedObject<TagType>&& object)
       {
        return TypedUniqueLocalObject<TagType>(PointerToValue<TypedObject<TagType>>(std::move(object)), TypedLocalObjectRefDeleter<TagType>(env));
       };


    template < class OutTagType, class InTagType >
    TypedObject<OutTagType> Cast(JNIEnv& env, const TypedObject<InTagType>& object, const TypedClass<OutTagType>& clazz)
       {
        if (!object.IsInstanceOf(env, clazz))
           {
            ThrowNew(env, FindClass(env, "java/lang/ClassCastException"));
           }
        return Object<OutTagType>(object.Get());
       }
   }
