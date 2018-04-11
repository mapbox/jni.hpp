#include "test.hpp"

#include <jni/jni.hpp>

#include <cassert>

static void TestGetVersion()
   {
    static jint version = 0x00010001;
    static TestEnv env;

    env.fns->GetVersion = [] (JNIEnv*) -> jint { return version; };
    assert(jni::GetVersion(env) == version);
   }

static void TestDefineClass()
   {
    static const char * className = "name";
    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> loader;
    static TestEnv env;

    env.fns->DefineClass = [] (JNIEnv*, const char* name, jobject, const jbyte*, jsize) -> jclass
       {
        if (name == className) return Unwrap(classValue.Ptr());
        env.exception = true;  return nullptr;
       };

    assert(classValue == jni::DefineClass(env, className, loader.Ref(), nullptr, 0));
    assert(Throws<jni::PendingJavaException>([] { jni::DefineClass(env, "other", loader.Ref(), nullptr, 0); }));
   }

static void TestFindClass()
   {
    static const char * className = "name";
    static Testable<jni::jclass> classValue;
    static TestEnv env;

    env.fns->FindClass = [] (JNIEnv*, const char* name) -> jclass
       {
        if (name == className) return Unwrap(classValue.Ptr());
        env.exception = true;  return nullptr;
       };

    assert(classValue == jni::FindClass(env, className));
    assert(Throws<jni::PendingJavaException>([] { jni::FindClass(env, "other"); }));
   }

static void TestNewAndDeleteGlobalRef()
   {
    static Testable<jni::jobject> objectValue;
    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> failureValue;
    static TestEnv env;

    env.fns->NewGlobalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return Unwrap(objectValue.Ptr());
        if (obj == Unwrap(classValue.Ptr()))
            return Unwrap(classValue.Ptr());
        env.exception = true;
        return nullptr;
       };

    env.fns->DeleteGlobalRef = [] (JNIEnv*, jobject obj) -> void
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return;
        env.exception = true;
       };

    assert(objectValue == *jni::NewGlobalRef(env, objectValue.Ptr()));
    assert(classValue == *jni::NewGlobalRef(env, classValue.Ptr()));
    assert(Throws<jni::PendingJavaException>([] { jni::NewGlobalRef(env, failureValue.Ptr()); }));
    assert(Throws<jni::PendingJavaException>(
       [] { jni::DeleteGlobalRef(env, jni::NewGlobalRef(env, failureValue.Ptr())); }));
   }

static void TestNewAndDeleteWeakGlobalRef()
   {
    static Testable<jni::jobject> objectValue;
    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> failureValue;
    static TestEnv env;

    env.fns->NewWeakGlobalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return Unwrap(objectValue.Ptr());
        if (obj == Unwrap(classValue.Ptr()))
            return Unwrap(classValue.Ptr());
        env.exception = true;
        return nullptr;
       };

    env.fns->DeleteWeakGlobalRef = [] (JNIEnv*, jobject obj) -> void
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return;
        env.exception = true;
       };

    assert(objectValue == *jni::NewWeakGlobalRef(env, objectValue.Ptr()));
    assert(classValue == *jni::NewWeakGlobalRef(env, classValue.Ptr()));
    assert(Throws<jni::PendingJavaException>([] { jni::NewWeakGlobalRef(env, failureValue.Ptr()); }));
    assert(Throws<jni::PendingJavaException>(
       [] { jni::DeleteWeakGlobalRef(env, jni::NewWeakGlobalRef(env, failureValue.Ptr())); }));
   }

static void TestNewAndDeleteLocalRef()
   {
    static Testable<jni::jobject> objectValue;
    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> failureValue;
    static TestEnv env;

    env.fns->NewLocalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return Unwrap(objectValue.Ptr());
        if (obj == Unwrap(classValue.Ptr()))
            return Unwrap(classValue.Ptr());
        env.exception = true;
        return nullptr;
       };

    env.fns->DeleteLocalRef = [] (JNIEnv*, jobject obj) -> void
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return;
        env.exception = true;
       };

    assert(objectValue == *jni::NewLocalRef(env, objectValue.Ptr()));
    assert(classValue == *jni::NewLocalRef(env, classValue.Ptr()));
    assert(Throws<jni::PendingJavaException>([] { jni::NewLocalRef(env, failureValue.Ptr()); }));
    assert(Throws<jni::PendingJavaException>(
       [] { jni::DeleteLocalRef(env, jni::NewLocalRef(env, failureValue.Ptr())); }));
   }

static void TestNewObject()
   {
    static Testable<jni::jclass> classValue;
    static Testable<jni::jclass> failureValue;
    static Testable<jni::jmethodID> methodValue;
    static Testable<jni::jobject> objectValue;
    static TestEnv env;

    env.fns->NewObjectV = [] (JNIEnv*, jclass clazz, jmethodID, va_list) -> jobject
       {
        if (clazz == Unwrap(classValue.Ptr()))
            return Unwrap(objectValue.Ptr());
        env.exception = true;
        return nullptr;
       };

    assert(objectValue == jni::NewObject(env, classValue.Ref(), methodValue.Ref()));
    assert(Throws<jni::PendingJavaException>([] { jni::NewObject(env, failureValue.Ref(), methodValue.Ref()); }));
   }

static void TestGetArrayLength()
   {
    static Testable<jni::jarray<jni::jobject>> arrayValue;
    static Testable<jni::jarray<jni::jobject>> failureValue;
    static TestEnv env;

    env.fns->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        if (array == jni::Unwrap(arrayValue.Ptr()))
            return 42;
        env.exception = true;
        return 0;
       };

    assert(42 == jni::GetArrayLength(env, arrayValue.Ref()));
    assert(Throws<jni::PendingJavaException>([] { jni::GetArrayLength(env, failureValue.Ref()); }));
   }

static void TestArrayElements()
   {
    static Testable<jni::jarray<jni::jboolean>> arrayValue;
    static TestEnv env;

    env.fns->GetBooleanArrayElements = [] (JNIEnv*, jbooleanArray, jboolean*) -> jboolean*
       {
        return nullptr;
       };

    env.fns->ReleaseBooleanArrayElements = [] (JNIEnv*, jbooleanArray, jboolean*, jint)
       {
       };

    auto result = jni::GetArrayElements<jni::jboolean>(env, arrayValue.Ref());
    jni::ReleaseArrayElements<jni::jboolean>(env, arrayValue.Ref(), std::get<0>(result).get());
    jni::ReleaseArrayElements<jni::jboolean>(env, arrayValue.Ref(), std::move(std::get<0>(result)));
   }

static void TestArrayRegion()
   {
    static Testable<jni::jarray<jni::jboolean>> arrayValue;
    static TestEnv env;

    env.fns->GetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray array, jsize start, jsize len, jboolean* buf)
       {
        assert(array == jni::Unwrap(arrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        *buf = jni::jni_true;
       };

    env.fns->SetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray array, jsize start, jsize len, const jboolean* buf)
       {
        assert(array == jni::Unwrap(arrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        assert(*buf == jni::jni_false);
       };

    jni::jboolean boolean = jni::jni_false;
    jni::GetArrayRegion<jni::jboolean>(env, arrayValue.Ref(), 0, 1, &boolean);
    assert(boolean == jni::jni_true);

    boolean = jni::jni_false;
    jni::SetArrayRegion<jni::jboolean>(env, arrayValue.Ref(), 0, 1, &boolean);
   }

namespace
   {
    void Method(jni::JNIEnv*, jni::jobject*) {}
    int StaticMethod(jni::JNIEnv*, jni::jclass*) { return 0; }

    struct Struct
       {
        static void Method(jni::JNIEnv*, jni::jobject*) {}
        static void StaticMethod(jni::JNIEnv*, jni::jclass*) {}
       };
   }

static void TestMakeNativeMethod()
   {
   // None of these should compile:
//    jni::MakeNativeMethod("name", "sig", Method                );
//    jni::MakeNativeMethod("name", "sig", StaticMethod          );
//    jni::MakeNativeMethod("name", "sig", &Method               );
//    jni::MakeNativeMethod("name", "sig", &StaticMethod         );
//    jni::MakeNativeMethod("name", "sig", &Struct::Method       );
//    jni::MakeNativeMethod("name", "sig", &Struct::StaticMethod );
//    jni::MakeNativeMethod("name", "sig", Struct() );

    jni::MakeNativeMethod< decltype(&Method),               &Method               >("name", "sig");
    jni::MakeNativeMethod< decltype(&StaticMethod),         &StaticMethod         >("name", "sig");
    jni::MakeNativeMethod< decltype(&Struct::Method),       &Struct::Method       >("name", "sig");
    jni::MakeNativeMethod< decltype(&Struct::StaticMethod), &Struct::StaticMethod >("name", "sig");
    jni::MakeNativeMethod("name", "sig", [] (jni::JNIEnv*, jni::jobject*) {});
    jni::MakeNativeMethod("name", "sig", [] (jni::JNIEnv*, jni::jclass*) {});
    jni::MakeNativeMethod("name", "sig", [] (jni::JNIEnv*, jni::jobject*) mutable {});
    jni::MakeNativeMethod("name", "sig", [] (jni::JNIEnv*, jni::jclass*) mutable {});
   }

int main()
   {
    TestGetVersion();

    TestDefineClass();
    TestFindClass();

    /*
    FromReflectedMethod
    FromReflectedField
    ToReflectedMethod
    ToReflectedField
    GetSuperclass
    IsAssignableFrom
    */

    /*
    Throw
    ThrowNew
    ExceptionCheck
    ExceptionOccurred
    ExceptionDescribe
    ExceptionClear
    FatalError
    PushLocalFrame
    PopLocalFrame
    */

    TestNewAndDeleteGlobalRef();
    TestNewAndDeleteWeakGlobalRef();
    TestNewAndDeleteLocalRef();

    /*
        jint        (*EnsureLocalCapacity)(JNIEnv*, jint);
        jboolean    (*IsSameObject)(JNIEnv*, jobject, jobject);
    */

    /*
    AllocObject
    */

    TestNewObject();

    /*
    GetObjectClass
    IsInstanceOf
    */

    /*
    GetMethodID
    CallMethod
    CallNonvirtualMethod
    GetFieldID
    GetField
    SetField
    */

    /*
    GetStaticMethodID
    CallStaticMethod
    GetStaticFieldID
    GetStaticField
    SetStaticField
    */

/*
    jstring     (*NewString)(JNIEnv*, const jchar*, jsize);
    jsize       (*GetStringLength)(JNIEnv*, jstring);
    const jchar* (*GetStringChars)(JNIEnv*, jstring, jboolean*);
    void        (*ReleaseStringChars)(JNIEnv*, jstring, const jchar*);
    jstring     (*NewStringUTF)(JNIEnv*, const char*);
    jsize       (*GetStringUTFLength)(JNIEnv*, jstring);
    const char* (*GetStringUTFChars)(JNIEnv*, jstring, jboolean*);
    void        (*ReleaseStringUTFChars)(JNIEnv*, jstring, const char*);
*/

    TestGetArrayLength();

    /*
    NewArray
    jobject     (*GetObjectArrayElement)(JNIEnv*, jobjectArray, jsize);
    void        (*SetObjectArrayElement)(JNIEnv*, jobjectArray, jsize, jobject);
    */

    TestArrayElements();
    TestArrayRegion();

    TestMakeNativeMethod();

    /*
        jint        (*RegisterNatives)(JNIEnv*, jclass, const JNINativeMethod*,
                            jint);
        jint        (*UnregisterNatives)(JNIEnv*, jclass);
        jint        (*MonitorEnter)(JNIEnv*, jobject);
        jint        (*MonitorExit)(JNIEnv*, jobject);
        jint        (*GetJavaVM)(JNIEnv*, JavaVM**);

        void        (*GetStringRegion)(JNIEnv*, jstring, jsize, jsize, jchar*);
        void        (*GetStringUTFRegion)(JNIEnv*, jstring, jsize, jsize, char*);

        void*       (*GetPrimitiveArrayCritical)(JNIEnv*, jarray, jboolean*);
        void        (*ReleasePrimitiveArrayCritical)(JNIEnv*, jarray, void*, jint);

        const jchar* (*GetStringCritical)(JNIEnv*, jstring, jboolean*);
        void        (*ReleaseStringCritical)(JNIEnv*, jstring, const jchar*);
    */

    /*
        jobject     (*NewDirectByteBuffer)(JNIEnv*, void*, jlong);
        void*       (*GetDirectBufferAddress)(JNIEnv*, jobject);
        jlong       (*GetDirectBufferCapacity)(JNIEnv*, jobject);
    */

    /*
        jobjectRefType (*GetObjectRefType)(JNIEnv*, jobject);
    */

    /*
    AttachCurrentThread
    DetachCurrentThread
    */

    return 0;
   }
