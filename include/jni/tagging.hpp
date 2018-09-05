#pragma once

#include <jni/traits.hpp>

#include <type_traits>

namespace jni
   {
    /*
        The interface for high-level references. Client code using the high-level API
        will most often work with values of this class template, using the following aliases:

            * Global<P>, a.k.a. Tagged<T, DefaultRefDeleter<&JNIEnv::DeleteGlobalRef>>,
            * Weak<P>, a.k.a. Tagged<T, DefaultRefDeleter<&JNIEnv::DeleteWeakGlobalRef>>,
            * Local<P>, a.k.a. Tagged<T, LocalRefDeleter>,

        where P is Object<>, Class<>, or Array<>.

        `Tagged` is an ownership class with a deletion policy that's parameterized both
        by the appropriate method to delete the reference (global, weak, or local) and
        (for global and weak references), a choice about how to obtain the JNIEnv that's
        necessary to call the deletion method. The default policy is to delete the reference
        using the same JNIEnv as was passed to the constructor, but in cases where the
        object may be deleted on a different thread (commonly, the Java finalizer thread),
        EnvGettingDeleter or EnvAttachingDeleter may be needed.

        Object<>, Class<>, or Array<> -- the underlying and inherited types used for
        the template parameter T -- are not publicly constructible or destructible. This
        is to ensure that code works only with ownership types which release the reference
        at an appropriate time. Our experience has shown that this is important even for
        local references; the default JVM cleanup behaviors for local references are not
        enough to ensure that the local reference table never overflows.

        In some cases C++ references -- Object<>&, const Class<>&, or Array<>& -- are used.
        For example, this is the case for receiving parameters passed to a native method
        implementation, reflecting the fact that JVM implementations prohibit explicitly
        releasing this form of local reference. You may also pass C++ references to code
        that does not need to take ownership. However, if you need to store or copy the
        reference, you will need to use a method such as `NewGlobalRef` that copies at the
        reference level -- `Tagged`, `Object<>`, etc., are not themselves copyable.
    */
    template < class T, class D >
    class Tagged : public T
       {
        private:
            D deleter;

            Tagged(const Tagged&) = delete;
            Tagged& operator=(const Tagged&) = delete;

        public:
            using Base = T;
            using UntaggedType = typename T::UntaggedType;

            explicit Tagged(std::nullptr_t ptr = nullptr)
               : T(ptr),
                 deleter() {}

            explicit Tagged(JNIEnv& env, UntaggedType* ptr)
               : T(ptr),
                 deleter(env) {}

            Tagged(Tagged&& other)
               : T(other.release()),
                 deleter(std::move(other.get_deleter())) {}

            template < class U >
            Tagged(Tagged<U, D>&& other, std::enable_if_t<std::is_convertible<const U&, const T&>::value>* = nullptr)
               : T(other.release()),
                 deleter(std::move(other.get_deleter())) {}

            ~Tagged()
               {
                reset();
               }

            Tagged& operator=(Tagged&& other)
               {
                reset(other.release());
                deleter = std::move(other.deleter);
                return *this;
               }

            void reset(UntaggedType* ptr = nullptr)
               {
                UntaggedType* current = T::Get();
                T::reset(ptr);
                if (current)
                   {
                    get_deleter()(current);
                   }
               }

            UntaggedType* release()
               {
                UntaggedType* current = T::Get();
                T::reset(nullptr);
                return current;
               }

                  D& get_deleter()         { return deleter; }
            const D& get_deleter() const   { return deleter; }
       };


    template < class T, template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
    using Global = Tagged< T, Deleter<&JNIEnv::DeleteGlobalRef> >;

    template < class T, template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
    using Weak = Tagged< T, Deleter<&JNIEnv::DeleteWeakGlobalRef> >;

    // Not parameterized by Deleter because local references should be short-lived enough
    // that DefaultRefDeleter suffices in all cases.
    template < class T >
    using Local = Tagged< T, DefaultRefDeleter<&JNIEnv::DeleteLocalRef> >;


    // Special case for JNI-provided input parameters to native methods, which apparently
    // should not be explicitly deleted (https://bugs.chromium.org/p/chromium/issues/detail?id=506850).
    struct NullDeleter
       {
        NullDeleter() = default;
        NullDeleter(JNIEnv&) {}
        void operator()(jobject*) const {}
       };

    template < class T >
    using Input = Tagged< T, NullDeleter >;


    // Attempt to promote a weak reference to a strong one. Returns an empty result
    // if the weak reference has expired.
    //
    // Beware that the semantics of JNI weak references are weaker than is typically
    // desired: a JNI weak reference may still be promoted to a non-null strong reference
    // even during finalization. Consider using jni::WeakReference<T> instead.
    template < template < RefDeletionMethod > class Deleter, class T, template < RefDeletionMethod > class WeakDeleter >
    Global<T, Deleter> NewGlobal(JNIEnv& env, const Weak<T, WeakDeleter>& t)
       {
        jobject* obj = Wrap<jobject*>(env.NewGlobalRef(Unwrap(t->Get())));
        CheckJavaException(env);
        return Global<T, Deleter>(env, obj);
       }

    template < class T >
    Global<T> NewGlobal(JNIEnv& env, const Weak<T>& t)
       {
        return NewGlobal<DefaultRefDeleter>(env, t);
       }


    // Attempt to promote a weak reference to a strong one. Returns an empty result
    // if the weak reference has expired.
    //
    // Beware that the semantics of JNI weak references are weaker than is typically
    // desired: a JNI weak reference may still be promoted to a non-null strong reference
    // even during finalization. Consider using jni::WeakReference<T> instead.
    template < class T, template < RefDeletionMethod > class WeakDeleter >
    Local<T> NewLocal(JNIEnv& env, const Weak<T, WeakDeleter>& t)
       {
        jobject* obj = Wrap<jobject*>(env.NewLocalRef(Unwrap(t->Get())));
        CheckJavaException(env);
        return Local<T>(env, obj);
       }


    template < class T >
    auto Tag(JNIEnv&, T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T, class U >
    auto Tag(JNIEnv& env, U* u)
       -> std::enable_if_t< !IsPrimitive<T>::value, Input<T> >
       {
        return Input<T>(env, u);
       }

    template < class T, class U >
    auto Tag(JNIEnv& env, U& u)
       -> std::enable_if_t< !IsPrimitive<T>::value, Input<T> >
       {
        return Input<T>(env, &u);
       }


    template < class T >
    auto Untag(T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T >
    auto Untag(const T& t)
       -> std::enable_if_t< !IsPrimitive<T>::value, decltype(t.Get()) >
       {
        return t.Get();
       }

    template < class T >
    struct UntaggedTypeTraits
       {
        using Type = decltype(Untag(std::declval<T>()));
       };

    template <>
    struct UntaggedTypeTraits<void>
       {
        using Type = void;
       };

    template < class T >
    using UntaggedType = typename UntaggedTypeTraits<T>::Type;


    template < class T >
    struct BaseTypeTraits
       {
        using Type = T;
       };

    template < class T >
    struct BaseTypeTraits< Local<T> >
       {
        using Type = T;
       };

    template < class T >
    using BaseType = typename BaseTypeTraits<T>::Type;


    template < class T >
    auto ReleaseLocal(T primitive)
       -> std::enable_if_t< IsPrimitive<T>::value, T >
       {
        return primitive;
       }

    template < class T >
    auto ReleaseLocal(Local<T>&& t)
       -> std::enable_if_t< !IsPrimitive<T>::value, decltype(t.release()) >
       {
        return t.release();
       }
   }
