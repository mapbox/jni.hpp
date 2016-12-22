#pragma once

#include <jni/functions.hpp>
#include <jni/object.hpp>
#include <jni/array.hpp>

#include <initializer_list>
#include <string>

namespace jni
   {
    template < class > struct TypeSignature;

    template <> struct TypeSignature< jboolean > { const char * operator()() const { return "Z"; } };
    template <> struct TypeSignature< jbyte    > { const char * operator()() const { return "B"; } };
    template <> struct TypeSignature< jchar    > { const char * operator()() const { return "C"; } };
    template <> struct TypeSignature< jshort   > { const char * operator()() const { return "S"; } };
    template <> struct TypeSignature< jint     > { const char * operator()() const { return "I"; } };
    template <> struct TypeSignature< jlong    > { const char * operator()() const { return "J"; } };
    template <> struct TypeSignature< jfloat   > { const char * operator()() const { return "F"; } };
    template <> struct TypeSignature< jdouble  > { const char * operator()() const { return "D"; } };
    template <> struct TypeSignature< void     > { const char * operator()() const { return "V"; } };

    template < class TheTag >
    struct TypeSignature< Object<TheTag> >
       {
        const char * operator()() const
           {
            static std::string value { std::string("L") + TheTag::Name() + ";" };
            return value.c_str();
           }
       };

    template < class E >
    struct TypeSignature< Array<E> >
       {
        const char * operator()() const
           {
            static std::string value { std::string("[") + TypeSignature<E>()() };
            return value.c_str();
           }
       };

    template < class R, class... Args >
    struct TypeSignature< R (Args...) >
       {
        private:
            template < class T > void DoNothingWith(const std::initializer_list<T>&) const {}

            std::string Compute() const
               {
                static std::string result("(");
                DoNothingWith<int>( { ( result += TypeSignature<Args>()(), 0 )... } );
                result += ")";
                result += TypeSignature<R>()();
                return result;
               }

        public:
            const char * operator()() const
               {
                static std::string result = Compute();
                return result.c_str();
               }
       };
   }
