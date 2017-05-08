#include <jni/types.hpp>

#ifdef _JAVASOFT_JNI_H_
using JNINativeInterface = JNINativeInterface_;
#endif

template < class T >
struct Testable
   {
    T& Ref() { return reinterpret_cast<T&>(*this); }
    T* Ptr() { return reinterpret_cast<T*>( this); }

    bool operator==(const T& other) { return &other == reinterpret_cast<T*>(this); }
   };

struct TestEnv : public jni::JNIEnv
   {
    TestEnv()
       : jni::JNIEnv { new JNINativeInterface },
         functions(const_cast<JNINativeInterface*>(jni::JNIEnv::functions))
       {
        functions->ExceptionCheck = [] (JNIEnv* env) -> jboolean
           {
            return reinterpret_cast<TestEnv*>(env)->exception ? JNI_TRUE : JNI_FALSE;
           };
       }

    ~TestEnv() { delete functions; }

    bool exception = false;
    JNINativeInterface* functions;
   };

template < class E, class Fn >
bool Throws(Fn&& fn)
   {
    try
       {
        fn();
        return false;
       }
    catch (const E&)
       {
        return true;
       }
   }
