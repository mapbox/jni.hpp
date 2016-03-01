#include "test.hpp"

#include <jni/jni.hpp>

#include <cassert>

static void TestGetVersion()
   {
    static jint version = 0x00010001;
    static TestEnv env;

    env.functions->GetVersion = [] (JNIEnv*) -> jint { return version; };
    assert(jni::GetVersion(env) == version);
   }

static void TestDefineClass()
   {
    static const char * className = "name";
    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> loader;
    static TestEnv env;

    env.functions->DefineClass = [] (JNIEnv*, const char* name, jobject, const jbyte*, jsize) -> jclass
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

    env.functions->FindClass = [] (JNIEnv*, const char* name) -> jclass
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

    env.functions->NewGlobalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        if (obj == Unwrap(objectValue.Ptr()))
            return Unwrap(objectValue.Ptr());
        if (obj == Unwrap(classValue.Ptr()))
            return Unwrap(classValue.Ptr());
        env.exception = true;
        return nullptr;
       };

    env.functions->DeleteGlobalRef = [] (JNIEnv*, jobject obj) -> void
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

static void TestNewObject()
   {
    static Testable<jni::jclass> classValue;
    static Testable<jni::jclass> failureValue;
    static Testable<jni::jmethodID> methodValue;
    static Testable<jni::jobject> objectValue;
    static TestEnv env;

    env.functions->NewObjectV = [] (JNIEnv*, jclass clazz, jmethodID, va_list) -> jobject
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

    env.functions->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        if (array == jni::Unwrap(arrayValue.Ptr()))
            return 42;
        env.exception = true;
        return 0;
       };

    assert(42 == jni::GetArrayLength(env, arrayValue.Ptr()));
    assert(Throws<jni::PendingJavaException>([] { jni::GetArrayLength(env, failureValue.Ptr()); }));
   }

static void TestArrayRegion()
   {
    static Testable<jni::jarray<jni::jboolean>> arrayValue;
    static TestEnv env;

    env.functions->GetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray array, jsize start, jsize len, jboolean* buf)
       {
        assert(array == jni::Unwrap(arrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        *buf = jni::jni_true;
       };

    env.functions->SetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray array, jsize start, jsize len, const jboolean* buf)
       {
        assert(array == jni::Unwrap(arrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        assert(*buf == jni::jni_false);
       };

    jni::jboolean boolean = jni::jni_false;
    jni::GetArrayRegion<jni::jboolean>(env, arrayValue.Ptr(), 0, 1, &boolean);
    assert(boolean == jni::jni_true);

    boolean = jni::jni_false;
    jni::SetArrayRegion<jni::jboolean>(env, arrayValue.Ptr(), 0, 1, &boolean);
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

    /*
        jobject     (*NewLocalRef)(JNIEnv*, jobject);
        void        (*DeleteLocalRef)(JNIEnv*, jobject);
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

    jboolean*   (*GetArrayElements)(JNIEnv*, jbooleanArray, jboolean*);
    void        (*ReleaseArrayElements)(JNIEnv*, jbooleanArray,
                        jboolean*, jint);
    */

    TestArrayRegion();

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

        jweak       (*NewWeakGlobalRef)(JNIEnv*, jobject);
        void        (*DeleteWeakGlobalRef)(JNIEnv*, jweak);
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
