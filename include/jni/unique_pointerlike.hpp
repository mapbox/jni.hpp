#pragma once

/*
    A class like std::unique_ptr<T, D>, but holds T by value; operator->() returns T*,
    not D::pointer; and deletion is performed via get_deleter()(current.Get()). These are
    the semantics needed by:

        * UniqueObject, a.k.a. UniquePointerlike<Object<TagType>, GlobalRefDeleter>,
        * UniqueWeakObject, a.k.a. UniquePointerlike<Object<TagType>, WeakGlobalRefDeleter>,
        * UniqueLocalObject, a.k.a. UniquePointerlike<Object<TagType>, LocalRefDeleter>,
        * and similar typedefs for Array<E>.
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
                pointerlike = std::move(t);
                if (current)
                   {
                    get_deleter()(current.Get());
                   }
               }

            T release()
               {
                T current = pointerlike;
                pointerlike = T();
                return current;
               }

            explicit operator bool() const { return pointerlike; }

            T* operator->() const          { return const_cast<T*>(&pointerlike); }
            T& operator*() const           { return const_cast<T&>(pointerlike); }

            T& get() const                 { return const_cast<T&>(pointerlike); }

                  D& get_deleter()         { return deleter; }
            const D& get_deleter() const   { return deleter; }
       };
   }
