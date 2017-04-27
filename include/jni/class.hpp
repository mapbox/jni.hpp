#pragma once

#include <jni/functions.hpp>
#include <jni/type.hpp>
#include <jni/tagging.hpp>
#include <jni/pointer_to_value.hpp>

namespace jni
   {
    template < class TagType > class TypedObject;
    template < class TagType, class... > class TypedConstructor;
    template < class TagType, class > class TypedField;
    template < class TagType, class > class TypedStaticField;
    template < class TagType, class > class TypedMethod;
    template < class TagType, class > class TypedStaticMethod;

    template < class TagType >
    class TypedClass;

    template < class TagType >
    class TypedClassDeleter;

    template < class TagType >
    using TypedUniqueClass = std::unique_ptr< const TypedClass<TagType>, TypedClassDeleter<TagType> >;

    template < class Tag >
    using Class = TypedClass< TypeFromTag<Tag> >;

    template <char... Chars>
    class TypedClass< Type<Chars...> >
       {
        private:
            jclass* clazz = nullptr;

        public:
            using TagType = Type<Chars...>;

            explicit TypedClass(std::nullptr_t = nullptr)
               {}

            explicit TypedClass(jclass& c)
               : clazz(&c)
               {}

            explicit operator bool() const { return clazz; }

            operator jclass&() const { return *clazz; }
            jclass& operator*() const { return *clazz; }
            jclass* Get() const { return clazz; }

            friend bool operator==( const TypedClass& a, const TypedClass& b )  { return a.Get() == b.Get(); }
            friend bool operator!=( const TypedClass& a, const TypedClass& b )  { return !( a == b ); }

            template < class... Args >
            TypedObject<TagType> New(JNIEnv& env, const TypedConstructor<TagType, Args...>& method, const Args&... args) const
               {
                return TypedObject<TagType>(&NewObject(env, *clazz, method, Untag(args)...));
               }

            template < class T >
            auto Get(JNIEnv& env, const TypedStaticField<TagType, T>& field) const
               -> std::enable_if_t< IsPrimitive<T>::value, T >
               {
                return jni::GetStaticField<T>(env, *clazz, field);
               }

            template < class T >
            auto Get(JNIEnv& env, const TypedStaticField<TagType, T>& field) const
               -> std::enable_if_t< !IsPrimitive<T>::value, T >
               {
                return T(reinterpret_cast<UntaggedType<T>>(jni::GetStaticField<jobject*>(env, *clazz, field)));
               }

            template < class T >
            auto Set(JNIEnv& env, const TypedStaticField<TagType, T>& field, T value) const
               -> std::enable_if_t< IsPrimitive<T>::value >
               {
                SetStaticField<T>(env, *clazz, field, value);
               }

            template < class T >
            auto Set(JNIEnv& env, const TypedStaticField<TagType, T>& field, const T& value) const
               -> std::enable_if_t< !IsPrimitive<T>::value >
               {
                SetStaticField<jobject*>(env, *clazz, field, value.Get());
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const TypedStaticMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< IsPrimitive<R>::value, R >
               {
                return CallStaticMethod<R>(env, *clazz, method, Untag(args)...);
               }

            template < class R, class... Args >
            auto Call(JNIEnv& env, const TypedStaticMethod<TagType, R (Args...)>& method, const Args&... args) const
               -> std::enable_if_t< !IsPrimitive<R>::value, R >
               {
                return R(reinterpret_cast<UntaggedType<R>>(CallStaticMethod<jobject*>(env, *clazz, method, Untag(args)...)));
               }

            template < class... Args >
            void Call(JNIEnv& env, const TypedStaticMethod<TagType, void (Args...)>& method, const Args&... args) const
               {
                CallStaticMethod<void>(env, *clazz, method, Untag(args)...);
               }

            static TypedClass Find(JNIEnv& env)
               {
                static constexpr const char name[] { Chars..., '\0' };
                return TypedClass(FindClass(env, name));
               }

            template < class... Args >
            TypedConstructor<TagType, Args...> GetConstructor(JNIEnv& env)
               {
                return TypedConstructor<TagType, Args...>(env, *this);
               }

            template < class T >
            TypedField<TagType, T> GetField(JNIEnv& env, const char* name)
               {
                return TypedField<TagType, T>(env, *this, name);
               }

            template < class T >
            TypedStaticField<TagType, T> GetStaticField(JNIEnv& env, const char* name)
               {
                return TypedStaticField<TagType, T>(env, *this, name);
               }

            template < class T >
            TypedMethod<TagType, T> GetMethod(JNIEnv& env, const char* name)
               {
                return TypedMethod<TagType, T>(env, *this, name);
               }

            template < class T >
            TypedStaticMethod<TagType, T> GetStaticMethod(JNIEnv& env, const char* name)
               {
                return TypedStaticMethod<TagType, T>(env, *this, name);
               }

            TypedUniqueClass<TagType> NewGlobalRef(JNIEnv& env) const
               {
                return Seize(env, TypedClass(*jni::NewGlobalRef(env, clazz).release()));
               }
       };

    template <char... Chars>
    class TypedClassDeleter< Type<Chars...> >
       {
        private:
            JNIEnv* env = nullptr;

        public:
            using pointer = PointerToValue< TypedClass< Type<Chars...> > >;

            TypedClassDeleter() = default;
            TypedClassDeleter(JNIEnv& e) : env(&e) {}

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
    TypedUniqueClass<TagType> Seize(JNIEnv& env, TypedClass<TagType>&& clazz)
       {
        return TypedUniqueClass<TagType>(PointerToValue<TypedClass<TagType>>(std::move(clazz)), TypedClassDeleter<TagType>(env));
       };
   }
