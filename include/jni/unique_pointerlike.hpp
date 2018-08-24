#pragma once

/*
    A class like std::unique_ptr<T, D>, but holds T by value; operator->() returns T*,
    not D::pointer; and deletion is performed via get_deleter()(current.Get()). These are
    the semantics needed by:

        * Global<P>, a.k.a. UniquePointerlike<T, GlobalRefDeleter>,
        * Weak<P>, a.k.a. UniquePointerlike<T, WeakGlobalRefDeleter>,
        * Local<P>, a.k.a. UniquePointerlike<T, LocalRefDeleter>,

    where P is Object<>, Class<>, or Array<>.
*/

namespace jni
   {
    template < class T, class D >
    class UniquePointerlike
       {
        private:
            T pointerlike;
            D deleter;

            UniquePointerlike(const UniquePointerlike&) = delete;
            UniquePointerlike& operator=(const UniquePointerlike&) = delete;

        public:
            UniquePointerlike()
               : pointerlike(),
                 deleter() {}

            explicit UniquePointerlike(T&& t, D&& d)
               : pointerlike(std::move(t)),
                 deleter(std::move(d)) {}

            UniquePointerlike(UniquePointerlike&& other)
               : pointerlike(other.release()),
                 deleter(std::move(other.get_deleter())) {}

            template < class U >
            UniquePointerlike(UniquePointerlike<U, D>&& other)
               : pointerlike(other.release()),
                 deleter(std::move(other.get_deleter())) {}

            ~UniquePointerlike()
               {
                reset();
               }

            UniquePointerlike& operator=(UniquePointerlike&& other)
               {
                reset(other.release());
                deleter = std::move(other.deleter);
                return *this;
               }

            void reset(T&& t = T())
               {
                T current = pointerlike;
                pointerlike.reset(t.Get());
                if (current)
                   {
                    get_deleter()(current.Get());
                   }
               }

            T release()
               {
                T current = pointerlike;
                pointerlike.reset(nullptr);
                return current;
               }

            explicit operator bool() const { return pointerlike; }

            T* operator->() const          { return const_cast<T*>(&pointerlike); }
            T& operator*() const           { return const_cast<T&>(pointerlike); }

            T& get() const                 { return const_cast<T&>(pointerlike); }

                  D& get_deleter()         { return deleter; }
            const D& get_deleter() const   { return deleter; }
       };


    template < class T, template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
    using Global = UniquePointerlike< T, Deleter<&JNIEnv::DeleteGlobalRef> >;

    template < template < RefDeletionMethod > class Deleter, class T >
    Global<T, Deleter> SeizeGlobal(JNIEnv& env, T&& t)
       {
        return Global<T, Deleter>(std::move(t), Deleter<&JNIEnv::DeleteGlobalRef>(env));
       }

    template < class T >
    Global<T> SeizeGlobal(JNIEnv& env, T&& t)
       {
        return SeizeGlobal<DefaultRefDeleter>(env, std::move(t));
       }


    template < class T, template < RefDeletionMethod > class Deleter = DefaultRefDeleter >
    using Weak = UniquePointerlike< T, Deleter<&JNIEnv::DeleteWeakGlobalRef> >;

    template < template < RefDeletionMethod > class Deleter, class T >
    Weak<T, Deleter> SeizeWeak(JNIEnv& env, T&& t)
       {
        return Weak<T, Deleter>(std::move(t), Deleter<&JNIEnv::DeleteWeakGlobalRef>(env));
       }

    template < class T >
    Weak<T> SeizeWeak(JNIEnv& env, T&& t)
       {
        return SeizeWeak<DefaultRefDeleter>(env, std::move(t));
       }


    // Not parameterized by Deleter because local references should be short-lived enough
    // that DefaultRefDeleter suffices in all cases.
    template < class T >
    using Local = UniquePointerlike< T, DefaultRefDeleter<&JNIEnv::DeleteLocalRef> >;

    template < class T >
    Local<T> SeizeLocal(JNIEnv& env, T&& t)
       {
        return Local<T>(std::move(t), DefaultRefDeleter<&JNIEnv::DeleteLocalRef>(env));
       }


    // Attempt to promote a weak reference to a strong one. Returns an empty result
    // if the weak reference has expired.
    template < template < RefDeletionMethod > class Deleter, class T, template < RefDeletionMethod > class WeakDeleter >
    Global<T, Deleter> NewGlobal(JNIEnv& env, const Weak<T, WeakDeleter>& t)
       {
        jobject* obj = Wrap<jobject*>(env.NewGlobalRef(Unwrap(t->Get())));
        CheckJavaException(env);
        return SeizeGlobal<Deleter>(env, T(obj));
       }

    template < class T >
    Global<T> NewGlobal(JNIEnv& env, const Weak<T>& t)
       {
        return NewGlobal<DefaultRefDeleter>(env, t);
       }


    // Attempt to promote a weak reference to a strong one. Returns an empty result
    // if the weak reference has expired.
    template < class T, template < RefDeletionMethod > class WeakDeleter >
    Local<T> NewLocal(JNIEnv& env, const Weak<T, WeakDeleter>& t)
       {
        jobject* obj = Wrap<jobject*>(env.NewLocalRef(Unwrap(t->Get())));
        CheckJavaException(env);
        return SeizeLocal(env, T(obj));
       }
   }
