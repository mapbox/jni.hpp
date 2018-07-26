#pragma once

#include <jni/jni.hpp>

namespace jni
   {

    template < class T >
    struct Name
       {
        struct Tag {};
       };

    template < class T >
    class Binding
       {
        using Tag = typename Name<T>::Tag;

        public:
            // Constructs from jobject* pointers directly.
            explicit Binding(UntaggedType<Object<Tag>> self_) : self(self_) {}

            Binding(Object<Tag> self_) : self(self_) {}

            static void jni_Register(jni::JNIEnv&) {}

            operator Object<Tag>() const { return self; }

        protected:
            static auto jni_Class(JNIEnv& env)
               {
                static auto javaClass = Class<Tag>::Find(env).NewGlobalRef(env).release();
                return *javaClass;
               }

            template < class... Args >
            static auto jni_New(JNIEnv& env, Args&&... args)
               {
                static const auto constructor = jni_Class(env).template GetConstructor<Args...>(env);
                return jni_Class(env).New(env, constructor, std::forward<Args>(args)...);
               }

            template < class Signature >
            static auto jni_GetMethod(JNIEnv& env, const char* name)
               {
                return jni_Class(env).template GetMethod<Signature>(env, name);
               }

            template < class M >
            struct jni_Bind;

            template < class R, class... Args >
            struct jni_Bind< R (Args...) > {
                template <R (*method)(JNIEnv&, Args...)>
                static auto StaticMethod(const char* name)
                   {
                    auto wrapper = [](JNIEnv* env, UntaggedType<Class<Tag>>, UntaggedType<Args>... args) -> UntaggedType<R>
                       {
                        try
                           {
                            return method(*env, AddTag<Args>(args)...);
                           }
                        catch (...)
                           {
                            ThrowJavaError(*env, std::current_exception());
                            return UntaggedType<R>();
                           }
                       };

                    using FunctionType = typename NativeMethodTraits<decltype(wrapper)>::Type;
                    return JNINativeMethod<FunctionType>{ name, TypeSignature<R (Args...)>()(), wrapper };
                   }

                template <R (T::*method)(JNIEnv&, Args...)>
                static auto Method(const char* name)
                   {
                    using Subject = Object<Tag>;
                    auto wrapper = [](JNIEnv* env, UntaggedType<Subject> subject, UntaggedType<Args>... args) -> UntaggedType<R>
                       {
                        try
                           {
                            return (T(AddTag<Subject>(*subject)).*method)(*env, AddTag<Args>(args)...);
                           }
                        catch (...)
                           {
                            ThrowJavaError(*env, std::current_exception());
                            return UntaggedType<R>();
                           }
                       };

                    using FunctionType = typename NativeMethodTraits<decltype(wrapper)>::Type;
                    return JNINativeMethod<FunctionType>{ name, TypeSignature<R(Args...)>()(), wrapper };
                   }
            };

        protected:
            Object<Tag> self;
       };



    template < class T >
    auto RemoveTag(const Binding<T>& t)
       {
        return Object<typename Name<T>::Tag>(t).Get();
       }


    template < class T >
    struct TypeSignature< T, std::enable_if_t<std::is_base_of<Binding<T>, T>::value> > {
        const char * operator()() const
           {
            return TypeSignature<Object<typename Name<T>::Tag>>()();
           }
        };

} // jni
