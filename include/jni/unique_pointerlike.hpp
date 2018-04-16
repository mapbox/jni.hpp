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

    template < class T > using Global = UniquePointerlike< T, GlobalRefDeleter >;
    template < class T > using Weak   = UniquePointerlike< T, WeakGlobalRefDeleter >;
    template < class T > using Local  = UniquePointerlike< T, LocalRefDeleter >;

    template < class T >
    Global<T> SeizeGlobal(JNIEnv& env, T&& t)
       {
        return Global<T>(std::move(t), GlobalRefDeleter(env));
       }

    template < class T >
    Weak<T> SeizeWeak(JNIEnv& env, T&& t)
       {
        return Weak<T>(std::move(t), WeakGlobalRefDeleter(env));
       }

    template < class T >
    Local<T> SeizeLocal(JNIEnv& env, T&& t)
       {
        return Local<T>(std::move(t), LocalRefDeleter(env));
       }
   }
