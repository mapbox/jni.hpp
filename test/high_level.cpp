#include "test.hpp"

#include <jni/jni.hpp>

#include <cassert>
#include <iostream>

namespace
   {
    struct Test { static constexpr auto Name() { return "mapbox/com/Test"; } };

    void Method(jni::JNIEnv&, jni::Object<Test>) {}
    int StaticMethod(jni::JNIEnv&, jni::Class<Test>) { return 0; }

    struct Peer
       {
        Peer() {}
        Peer(jni::JNIEnv&, jni::jboolean) {}
        jni::jboolean True(jni::JNIEnv&) { return jni::jni_true; }
        jni::jboolean False(jni::JNIEnv&) { return jni::jni_false; }
        void Void(jni::JNIEnv&, jni::jboolean b) { assert(b == jni::jni_true); }
        static void Static(jni::JNIEnv&, Peer&) {}
       };

    struct Base {};
    struct Derived : public Base {};
   }

int main()
   {
    /// TypeSignature

    assert(jni::TypeSignature< jni::jboolean >()() == "Z");
    assert(jni::TypeSignature< jni::jbyte    >()() == "B");
    assert(jni::TypeSignature< jni::jchar    >()() == "C");
    assert(jni::TypeSignature< jni::jshort   >()() == "S");
    assert(jni::TypeSignature< jni::jint     >()() == "I");
    assert(jni::TypeSignature< jni::jlong    >()() == "J");
    assert(jni::TypeSignature< jni::jfloat   >()() == "F");
    assert(jni::TypeSignature< jni::jdouble  >()() == "D");
    assert(jni::TypeSignature< void          >()() == "V");

    assert(jni::TypeSignature< jni::Object<>  >()() == std::string("Ljava/lang/Object;"));
    assert(jni::TypeSignature< jni::String    >()() == std::string("Ljava/lang/String;"));

    assert(jni::TypeSignature< jni::Array<jni::jboolean> >()() == std::string("[Z"));
    assert(jni::TypeSignature< jni::Array<jni::String>   >()() == std::string("[Ljava/lang/String;"));

    assert(jni::TypeSignature< void () >()() == std::string("()V"));
    assert(jni::TypeSignature< jni::jboolean () >()() == std::string("()Z"));
    assert(jni::TypeSignature< void (jni::jboolean, jni::jbyte, jni::jchar) >()() == std::string("(ZBC)V"));

    struct String { static constexpr const char * Name() { return "java/lang/String"; } };
    assert(jni::TypeSignature< jni::Object<String> >()() == std::string("Ljava/lang/String;"));
    assert(jni::TypeSignature< void (jni::Object<String>) >()() == std::string("(Ljava/lang/String;)V"));
    assert(jni::TypeSignature< jni::Object<String> (void) >()() == std::string("()Ljava/lang/String;"));


    /// Class

    static TestEnv env;

    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> objectValue;
    static Testable<jni::jstring> stringValue;

    env.functions->FindClass = [] (JNIEnv*, const char* name) -> jclass
       {
        assert(name == Test::Name());
        return Unwrap(classValue.Ptr());
       };

    jni::Class<Test> testClass { jni::Class<Test>::Find(env) };
    assert(classValue == testClass);

    static bool calledNewGlobalRef = false;
    static bool calledNewWeakGlobalRef = false;
    static bool calledNewLocalRef = false;

    env.functions->NewGlobalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        calledNewGlobalRef = true;
        return obj;
       };

    env.functions->DeleteGlobalRef = [] (JNIEnv*, jobject) -> void
       {
       };

    env.functions->NewWeakGlobalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        calledNewWeakGlobalRef = true;
        return obj;
       };

    env.functions->DeleteWeakGlobalRef = [] (JNIEnv*, jobject) -> void
       {
       };

    env.functions->NewLocalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        calledNewLocalRef = true;
        return obj;
       };

    env.functions->DeleteLocalRef = [] (JNIEnv*, jobject) -> void
       {
       };

    testClass.NewGlobalRef(env);
    assert(calledNewGlobalRef);


    /// Object

    jni::Object<Test> object { objectValue.Ptr() };
    object.NewGlobalRef(env);
    object.NewWeakGlobalRef(env);
    object.NewLocalRef(env);

    jni::String string { stringValue.Ptr() };
    string.NewGlobalRef(env);
    string.NewWeakGlobalRef(env);
    string.NewLocalRef(env);

    jni::Object<Derived> derived;
    jni::Object<Base> base(derived);
    base = derived;
    (void)[] () -> jni::Object<Base> { return jni::Object<Derived>(); };

    /// Constructor

    static Testable<jni::jmethodID> defaultConstructorMethodID;
    static Testable<jni::jmethodID> booleanConstructorMethodID;
    static Testable<jni::jmethodID> objectConstructorMethodID;

    env.functions->GetMethodID = [] (JNIEnv*, jclass, const char* name, const char* sig) -> jmethodID
       {
        assert(name == std::string("<init>"));

        if (sig == std::string("()V"))
           {
            return jni::Unwrap(defaultConstructorMethodID.Ptr());
           }
        else if (sig == std::string("(Z)V"))
           {
            return jni::Unwrap(booleanConstructorMethodID.Ptr());
           }
        else if (sig == std::string("(Lmapbox/com/Test;)V"))
           {
            return jni::Unwrap(objectConstructorMethodID.Ptr());
           }
        else
           {
            assert(((void)"unexpected signature", false));
           }
       };

    env.functions->NewObjectV = [] (JNIEnv*, jclass, jmethodID methodID, va_list args) -> jobject
       {
        if (methodID == jni::Unwrap(defaultConstructorMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(booleanConstructorMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(objectConstructorMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }

        va_end(args);
        return jni::Unwrap(objectValue.Ptr());
       };

    auto defaultNew = testClass.GetConstructor(env);
    auto booleanNew = testClass.GetConstructor<jni::jboolean>(env);
    auto objectNew  = testClass.GetConstructor<jni::Object<Test>>(env);

    assert(testClass.New(env, defaultNew).Get() == object.Get());
    assert(testClass.New(env, booleanNew, jni::jni_true).Get() == object.Get());
    assert(testClass.New(env, objectNew, object).Get() == object.Get());


    /// StaticField

    static Testable<jni::jfieldID> booleanFieldID;
    static Testable<jni::jfieldID> objectFieldID;
    static Testable<jni::jfieldID> stringFieldID;
    static const char * booleanFieldName = "boolean";
    static const char * objectFieldName = "object";
    static const char * stringFieldName = "string";

    env.functions->GetStaticFieldID = [] (JNIEnv*, jclass, const char* name, const char* sig) -> jfieldID
       {
        if (name == booleanFieldName)
           {
            assert(sig == std::string("Z"));
            return jni::Unwrap(booleanFieldID.Ptr());
           }
        else if (name == objectFieldName)
           {
            assert(sig == std::string("Lmapbox/com/Test;"));
            return jni::Unwrap(objectFieldID.Ptr());
           }
        else if (name == stringFieldName)
           {
            assert(sig == std::string("Ljava/lang/String;"));
            return jni::Unwrap(stringFieldID.Ptr());
           }
        else
           {
            abort();
           }
       };

    env.functions->GetStaticBooleanField = [] (JNIEnv*, jclass clazz, jfieldID field) -> jboolean
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));
        assert(field == jni::Unwrap(booleanFieldID.Ptr()));
        return JNI_TRUE;
       };

    env.functions->GetStaticObjectField = [] (JNIEnv*, jclass clazz, jfieldID field) -> jobject
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));
        assert(field == jni::Unwrap(objectFieldID.Ptr()));
        return jni::Unwrap(objectValue.Ptr());
       };

    env.functions->SetStaticBooleanField = [] (JNIEnv*, jclass clazz, jfieldID field, jboolean value)
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));
        assert(field == jni::Unwrap(booleanFieldID.Ptr()));
        assert(value == JNI_FALSE);
       };

    env.functions->SetStaticObjectField = [] (JNIEnv*, jclass clazz, jfieldID field, jobject value)
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));
        assert(field == jni::Unwrap(objectFieldID.Ptr()));
        assert(value == jni::Unwrap(objectValue.Ptr()));
       };

    auto booleanStaticField = testClass.GetStaticField<jni::jboolean>(env, booleanFieldName);
    auto objectStaticField  = testClass.GetStaticField<jni::Object<Test>>(env, objectFieldName);

    assert(testClass.Get(env, booleanStaticField) == jni::jni_true);
    assert(testClass.Get(env, objectStaticField).Get() == object.Get());
    testClass.Set(env, booleanStaticField, jni::jni_false);
    testClass.Set(env, objectStaticField, object);


    /// Field

    env.functions->GetFieldID = env.functions->GetStaticFieldID; // Reuse from above

    env.functions->GetBooleanField = [] (JNIEnv*, jobject obj, jfieldID field) -> jboolean
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));
        assert(field == jni::Unwrap(booleanFieldID.Ptr()));
        return JNI_TRUE;
       };

    env.functions->GetObjectField = [] (JNIEnv*, jobject obj, jfieldID field) -> jobject
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));
        if (field == jni::Unwrap(objectFieldID.Ptr()))
           {
            return jni::Unwrap(objectValue.Ptr());
           }
        else if (field == jni::Unwrap(stringFieldID.Ptr()))
           {
            return jni::Unwrap(stringValue.Ptr());
           }
        else
           {
            abort();
           }
       };

    env.functions->SetBooleanField = [] (JNIEnv*, jobject obj, jfieldID field, jboolean value)
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));
        assert(field == jni::Unwrap(booleanFieldID.Ptr()));
        assert(value == JNI_FALSE);
       };

    env.functions->SetObjectField = [] (JNIEnv*, jobject obj, jfieldID field, jobject value)
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));
        if (field == jni::Unwrap(objectFieldID.Ptr()))
           {
            assert(value == jni::Unwrap(objectValue.Ptr()));
           }
        else if (field == jni::Unwrap(stringFieldID.Ptr()))
           {
            assert(value == jni::Unwrap(stringValue.Ptr()));
           }
        else
           {
            abort();
           }
       };

    auto booleanField = testClass.GetField<jni::jboolean>(env, booleanFieldName);
    auto objectField  = testClass.GetField<jni::Object<Test>>(env, objectFieldName);
    auto stringField  = testClass.GetField<jni::String>(env, stringFieldName);

    assert(object.Get(env, booleanField) == true);
    assert(object.Get(env, objectField).Get() == object.Get());
    assert(object.Get(env, stringField).Get() == string.Get());

    object.Set(env, booleanField, jni::jni_false);
    object.Set(env, objectField, object);
    object.Set(env, stringField, string);


    /// StaticMethod

    static Testable<jni::jmethodID> voidVoidMethodID;
    static Testable<jni::jmethodID> voidBooleanMethodID;
    static Testable<jni::jmethodID> voidObjectMethodID;
    static Testable<jni::jmethodID> booleanVoidMethodID;
    static Testable<jni::jmethodID> booleanBooleanMethodID;
    static Testable<jni::jmethodID> booleanObjectMethodID;
    static Testable<jni::jmethodID> objectVoidMethodID;
    static Testable<jni::jmethodID> objectBooleanMethodID;
    static Testable<jni::jmethodID> objectObjectMethodID;
    static const char * voidMethodName = "returnVoid";
    static const char * booleanMethodName = "returnBoolean";
    static const char * objectMethodName = "returnObject";

    env.functions->GetStaticMethodID = [] (JNIEnv*, jclass, const char* name, const char* sig) -> jmethodID
       {
        if (sig == std::string("()V"))
           {
            assert(name == std::string("returnVoid"));
            return jni::Unwrap(voidVoidMethodID.Ptr());
           }
        else if (sig == std::string("(Z)V"))
           {
            assert(name == std::string("returnVoid"));
            return jni::Unwrap(voidBooleanMethodID.Ptr());
           }
        else if (sig == std::string("(Lmapbox/com/Test;)V"))
           {
            assert(name == std::string("returnVoid"));
            return jni::Unwrap(voidObjectMethodID.Ptr());
           }
        else if (sig == std::string("()Z"))
           {
            assert(name == std::string("returnBoolean"));
            return jni::Unwrap(booleanVoidMethodID.Ptr());
           }
        else if (sig == std::string("(Z)Z"))
           {
            assert(name == std::string("returnBoolean"));
            return jni::Unwrap(booleanBooleanMethodID.Ptr());
           }
        else if (sig == std::string("(Lmapbox/com/Test;)Z"))
           {
            assert(name == std::string("returnBoolean"));
            return jni::Unwrap(booleanObjectMethodID.Ptr());
           }
        else if (sig == std::string("()Lmapbox/com/Test;"))
           {
            assert(name == std::string("returnObject"));
            return jni::Unwrap(objectVoidMethodID.Ptr());
           }
        else if (sig == std::string("(Z)Lmapbox/com/Test;"))
           {
            assert(name == std::string("returnObject"));
            return jni::Unwrap(objectBooleanMethodID.Ptr());
           }
        else if (sig == std::string("(Lmapbox/com/Test;)Lmapbox/com/Test;"))
           {
            assert(name == std::string("returnObject"));
            return jni::Unwrap(objectObjectMethodID.Ptr());
           }
        else
           {
            assert(((void)"unexpected signature", false));
           }
       };

    env.functions->CallStaticVoidMethodV = [] (JNIEnv*, jclass clazz, jmethodID methodID, va_list args) -> void
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));

        if (methodID == jni::Unwrap(voidVoidMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(voidBooleanMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(voidObjectMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }
        else
           {
            assert(((void)"unexpected method", false));
           }

        va_end(args);
       };

    auto voidVoidStaticMethod    = testClass.GetStaticMethod<void ()>(env, voidMethodName);
    auto voidBooleanStaticMethod = testClass.GetStaticMethod<void (jni::jboolean)>(env, voidMethodName);
    auto voidObjectStaticMethod  = testClass.GetStaticMethod<void (jni::Object<Test>)>(env, voidMethodName);

    testClass.Call(env, voidVoidStaticMethod);
    testClass.Call(env, voidBooleanStaticMethod, jni::jni_true);
    testClass.Call(env, voidObjectStaticMethod, object);

    env.functions->CallStaticBooleanMethodV = [] (JNIEnv*, jclass clazz, jmethodID methodID, va_list args) -> jboolean
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));

        if (methodID == jni::Unwrap(booleanVoidMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(booleanBooleanMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(booleanObjectMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }
        else
           {
            assert(((void)"unexpected method", false));
           }

        va_end(args);
        return JNI_TRUE;
       };

    auto booleanVoidStaticMethod    = testClass.GetStaticMethod<jni::jboolean ()>(env, booleanMethodName);
    auto booleanBooleanStaticMethod = testClass.GetStaticMethod<jni::jboolean (jni::jboolean)>(env, booleanMethodName);
    auto booleanObjectStaticMethod  = testClass.GetStaticMethod<jni::jboolean (jni::Object<Test>)>(env, booleanMethodName);

    assert(testClass.Call(env, booleanVoidStaticMethod) == jni::jni_true);
    assert(testClass.Call(env, booleanBooleanStaticMethod, jni::jni_true) == jni::jni_true);
    assert(testClass.Call(env, booleanObjectStaticMethod, object) == jni::jni_true);

    env.functions->CallStaticObjectMethodV = [] (JNIEnv*, jclass clazz, jmethodID methodID, va_list args) -> jobject
       {
        assert(clazz == jni::Unwrap(classValue.Ptr()));

        if (methodID == jni::Unwrap(objectVoidMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(objectBooleanMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(objectObjectMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }
        else
           {
            assert(((void)"unexpected method", false));
           }

        va_end(args);
        return jni::Unwrap(objectValue.Ptr());
       };

    auto objectVoidStaticMethod    = testClass.GetStaticMethod<jni::Object<Test> ()>(env, objectMethodName);
    auto objectBooleanStaticMethod = testClass.GetStaticMethod<jni::Object<Test> (jni::jboolean)>(env, objectMethodName);
    auto objectObjectStaticMethod  = testClass.GetStaticMethod<jni::Object<Test> (jni::Object<Test>)>(env, objectMethodName);

    assert(testClass.Call(env, objectVoidStaticMethod).Get() == object.Get());
    assert(testClass.Call(env, objectBooleanStaticMethod, jni::jni_true).Get() == object.Get());
    assert(testClass.Call(env, objectObjectStaticMethod, object).Get() == object.Get());


    /// Method

    env.functions->GetMethodID = env.functions->GetStaticMethodID; // Reuse from above

    env.functions->CallVoidMethodV = [] (JNIEnv*, jobject obj, jmethodID methodID, va_list args) -> void
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));

        if (methodID == jni::Unwrap(voidVoidMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(voidBooleanMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(voidObjectMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }
        else
           {
            assert(((void)"unexpected method", false));
           }

        va_end(args);
       };

    auto voidVoidMethod    = testClass.GetMethod<void ()>(env, voidMethodName);
    auto voidBooleanMethod = testClass.GetMethod<void (jni::jboolean)>(env, voidMethodName);
    auto voidObjectMethod  = testClass.GetMethod<void (jni::Object<Test>)>(env, voidMethodName);

    object.Call(env, voidVoidMethod);
    object.Call(env, voidBooleanMethod, jni::jni_true);
    object.Call(env, voidObjectMethod, object);

    env.functions->CallBooleanMethodV = [] (JNIEnv*, jobject obj, jmethodID methodID, va_list args) -> jboolean
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));

        if (methodID == jni::Unwrap(booleanVoidMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(booleanBooleanMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(booleanObjectMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }
        else
           {
            assert(((void)"unexpected method", false));
           }

        va_end(args);
        return JNI_TRUE;
       };

    auto booleanVoidMethod    = testClass.GetMethod<jni::jboolean ()>(env, booleanMethodName);
    auto booleanBooleanMethod = testClass.GetMethod<jni::jboolean (jni::jboolean)>(env, booleanMethodName);
    auto booleanObjectMethod  = testClass.GetMethod<jni::jboolean (jni::Object<Test>)>(env, booleanMethodName);

    assert(object.Call(env, booleanVoidMethod) == jni::jni_true);
    assert(object.Call(env, booleanBooleanMethod, jni::jni_true) == jni::jni_true);
    assert(object.Call(env, booleanObjectMethod, object) == jni::jni_true);

    env.functions->CallObjectMethodV = [] (JNIEnv*, jobject obj, jmethodID methodID, va_list args) -> jobject
       {
        assert(obj == jni::Unwrap(objectValue.Ptr()));

        if (methodID == jni::Unwrap(objectVoidMethodID.Ptr()))
           {
           }
        else if (methodID == jni::Unwrap(objectBooleanMethodID.Ptr()))
           {
            assert(va_arg(args, int) == JNI_TRUE);
           }
        else if (methodID == jni::Unwrap(objectObjectMethodID.Ptr()))
           {
            assert(va_arg(args, jobject) == jni::Unwrap(objectValue.Ptr()));
           }
        else
           {
            assert(((void)"unexpected method", false));
           }

        va_end(args);
        return jni::Unwrap(objectValue.Ptr());
       };

    auto objectVoidMethod    = testClass.GetMethod<jni::Object<Test> ()>(env, objectMethodName);
    auto objectBooleanMethod = testClass.GetMethod<jni::Object<Test> (jni::jboolean)>(env, objectMethodName);
    auto objectObjectMethod  = testClass.GetMethod<jni::Object<Test> (jni::Object<Test>)>(env, objectMethodName);

    assert(object.Call(env, objectVoidMethod).Get() == object.Get());
    assert(object.Call(env, objectBooleanMethod, jni::jni_true).Get() == object.Get());
    assert(object.Call(env, objectObjectMethod, object).Get() == object.Get());


    /// Primitive Array

    static Testable<jni::jarray<jni::jboolean>> booleanArrayValue;

    env.functions->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        assert(array == jni::Unwrap(booleanArrayValue.Ptr()));
        return 42;
       };

    env.functions->GetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray, jsize, jsize, jboolean* buf)
       {
        *buf = jni::jni_true;
       };

    jni::Array<jni::jboolean> booleanArray { booleanArrayValue.Ptr() };
    assert(booleanArray.Length(env) == 42);
    assert(booleanArray.Get(env, 0) == jni::jni_true);

    static Testable<jni::jarray<jni::jbyte>> byteArrayValue;

    env.functions->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        assert(array == jni::Unwrap(byteArrayValue.Ptr()));
        return 42;
       };

    env.functions->GetByteArrayRegion = [] (JNIEnv*, jbyteArray, jsize, jsize, jbyte* buf)
       {
        *buf = 's';
       };

    jni::Array<jni::jbyte> byteArray { byteArrayValue.Ptr() };
    assert(byteArray.Length(env) == 42);
    assert(byteArray.Get(env, 0) == 's');


    /// Object Array

    static Testable<jni::jarray<jni::jobject>> objectArrayValue;

    env.functions->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        assert(array == jni::Unwrap(objectArrayValue.Ptr()));
        return 42;
       };

    env.functions->GetObjectArrayElement = [] (JNIEnv*, jobjectArray, jsize) -> jobject
       {
        return jni::Unwrap(objectValue.Ptr());
       };

    jni::Array<jni::Object<Test>> objectArray { objectArrayValue.Ptr() };
    assert(objectArray.Length(env) == 42);
    assert(objectArray.Get(env, 0).Get() == object.Get());


    /// NativeMethod

    auto TestNativeMethod = [&] (auto& objectOrClass)
       {
        using ObjectOrClass = typename std::decay<decltype(objectOrClass)>::type;

        auto voidVoid = jni::MakeNativeMethod("voidVoid", [] (JNIEnv&, ObjectOrClass) {});
        assert(voidVoid.name == std::string("voidVoid"));
        assert(voidVoid.signature == std::string("()V"));
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(voidVoid.fnPtr)(&env, nullptr);

        auto trueVoid = jni::MakeNativeMethod("true", [] (JNIEnv&, ObjectOrClass) { return jni::jni_true; });
        assert(trueVoid.signature == std::string("()Z"));
        assert(reinterpret_cast<jboolean (*)(JNIEnv*, jobject)>(trueVoid.fnPtr)(&env, nullptr) == JNI_TRUE);

        auto falseVoid = jni::MakeNativeMethod("false", [] (JNIEnv&, ObjectOrClass) { return jni::jni_false; });
        assert(falseVoid.signature == std::string("()Z"));
        assert(reinterpret_cast<jboolean (*)(JNIEnv*, jobject)>(falseVoid.fnPtr)(&env, nullptr) == JNI_FALSE);

        auto voidTrue = jni::MakeNativeMethod("voidTrue", [] (JNIEnv&, ObjectOrClass, jni::jboolean b) { assert( b); });
        assert(voidTrue.signature == std::string("(Z)V"));
        reinterpret_cast<jboolean (*)(JNIEnv*, jobject, jboolean)>(voidTrue.fnPtr)(&env, nullptr, JNI_TRUE);

        auto voidFalse = jni::MakeNativeMethod("voidFalse", [] (JNIEnv&, ObjectOrClass, jni::jboolean b) { assert(!b); });
        assert(voidFalse.signature == std::string("(Z)V"));
        reinterpret_cast<jboolean (*)(JNIEnv*, jobject, jboolean)>(voidFalse.fnPtr)(&env, nullptr, JNI_FALSE);

        auto voidObject = jni::MakeNativeMethod("voidObject", [] (JNIEnv&, ObjectOrClass, jni::Object<Test>) {});
        assert(voidObject.signature == std::string("(Lmapbox/com/Test;)V"));
        reinterpret_cast<jboolean (*)(JNIEnv*, jobject, jobject)>(voidObject.fnPtr)(&env, nullptr, nullptr);

        auto objectVoid = jni::MakeNativeMethod("objectVoid", [] (JNIEnv&, ObjectOrClass) -> jni::Object<Test> { return jni::Object<Test>(); });
        assert(objectVoid.signature == std::string("()Lmapbox/com/Test;"));
        assert(reinterpret_cast<jobject (*)(JNIEnv*, jobject)>(objectVoid.fnPtr)(&env, nullptr) == nullptr);

        auto objectObject = jni::MakeNativeMethod("objectObject", [] (JNIEnv&, ObjectOrClass, jni::Object<Test>) -> jni::Object<Test> { return jni::Object<Test>(); });
        assert(objectObject.signature == std::string("(Lmapbox/com/Test;)Lmapbox/com/Test;"));
        assert(reinterpret_cast<jobject (*)(JNIEnv*, jobject, jobject)>(objectObject.fnPtr)(&env, nullptr, nullptr) == nullptr);


        static const char* lastExceptionMessage = nullptr;
        static Testable<jni::jclass> errorClassValue;

        env.functions->FindClass = [] (JNIEnv*, const char* name) -> jclass
           {
            assert(name == std::string("java/lang/Error"));
            return Unwrap(errorClassValue.Ptr());
           };

        env.functions->ThrowNew = [] (JNIEnv*, ::jclass clazz, const char* message) -> jint
           {
            assert(clazz == Unwrap(errorClassValue.Ptr()));
            lastExceptionMessage = message;
            return 0;
           };

        auto throwsException = jni::MakeNativeMethod("throwsException", [] (JNIEnv&, ObjectOrClass) { throw std::runtime_error("test"); });
        lastExceptionMessage = nullptr;
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(throwsException.fnPtr)(&env, nullptr);
        assert(lastExceptionMessage == std::string("test"));

        auto throwsUnknown = jni::MakeNativeMethod("throwsUnknown", [] (JNIEnv&, ObjectOrClass) { throw Test(); });
        lastExceptionMessage = nullptr;
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(throwsUnknown.fnPtr)(&env, nullptr);
        assert(lastExceptionMessage == std::string("unknown native exception"));

        auto javaException = jni::MakeNativeMethod("javaException", [] (JNIEnv&, ObjectOrClass) { jni::ThrowNew(env, jni::FindClass(env, "java/lang/Error"), "Java exception"); });
        lastExceptionMessage = nullptr;
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(javaException.fnPtr)(&env, nullptr);
        assert(lastExceptionMessage == std::string("Java exception"));
       };

    TestNativeMethod(object);
    TestNativeMethod(testClass);


    /// Make

    env.functions->NewString = [] (JNIEnv*, const jchar*, jsize) -> jstring
       {
        return jni::Unwrap(stringValue.Ptr());
       };

    env.functions->GetStringLength = [] (JNIEnv*, jstring str) -> jsize
       {
        assert(str == jni::Unwrap(stringValue.Ptr()));
        return 5;
       };

    env.functions->GetStringRegion = [] (JNIEnv*, jstring str, jsize start, jsize len, jchar* buf)
       {
        assert(str == jni::Unwrap(stringValue.Ptr()));
        assert(start == 0);
        assert(len == 5);
        std::u16string(u"hello").copy(jni::Wrap<char16_t*>(buf), jni::Wrap<std::size_t>(len));
       };

    assert(jni::Make<std::string>(env, jni::Make<jni::String>(env, "hello")) == "hello");
    assert(jni::Make<std::u16string>(env, jni::Make<jni::String>(env, u"hello")) == u"hello");


    env.functions->NewBooleanArray = [] (JNIEnv*, jsize) -> jbooleanArray
       {
        return jni::Unwrap(booleanArrayValue.Ptr());
       };

    env.functions->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        assert(array == jni::Unwrap(booleanArrayValue.Ptr()));
        return 1;
       };

    env.functions->GetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray array, jsize start, jsize len, jboolean* buf)
       {
        assert(array == jni::Unwrap(booleanArrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        *buf = jni::jni_true;
       };

    env.functions->SetBooleanArrayRegion = [] (JNIEnv*, jbooleanArray array, jsize start, jsize len, const jboolean* buf)
       {
        assert(array == jni::Unwrap(booleanArrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        assert(*buf == jni::jni_true);
       };

    std::vector<jboolean> vec = { jni::jni_true };
    assert(jni::Make<std::vector<jboolean>>(env, jni::Make<jni::Array<jni::jboolean>>(env, vec)) == vec);


    env.functions->NewByteArray = [] (JNIEnv*, jsize) -> jbyteArray
       {
        return jni::Unwrap(byteArrayValue.Ptr());
       };

    env.functions->GetArrayLength = [] (JNIEnv*, jarray array) -> jsize
       {
        assert(array == jni::Unwrap(byteArrayValue.Ptr()));
        return 1;
       };

    env.functions->GetByteArrayRegion = [] (JNIEnv*, jbyteArray array, jsize start, jsize len, jbyte* buf)
       {
        assert(array == jni::Unwrap(byteArrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        *buf = 's';
       };

    env.functions->SetByteArrayRegion = [] (JNIEnv*, jbyteArray array, jsize start, jsize len, const jbyte* buf)
       {
        assert(array == jni::Unwrap(byteArrayValue.Ptr()));
        assert(start == 0);
        assert(len == 1);
        assert(*buf == 's');
       };

    std::vector<jbyte> byte_vec = { 's' };
    assert(jni::Make<std::vector<jbyte>>(env, jni::Make<jni::Array<jni::jbyte>>(env, byte_vec)) == byte_vec);
    std::string str("s");
    assert(jni::Make<std::string>(env, jni::Make<jni::Array<jni::jbyte>>(env, str)) == str);


    jni::MakeNativeMethod<decltype(&Method), &Method>("name");
    jni::MakeNativeMethod<decltype(&StaticMethod), &StaticMethod>("name");


    static JNINativeMethod methods[6];

    static Peer peerInstance;

    env.functions->GetFieldID = [] (JNIEnv*, jclass clazz, const char* name, const char* sig) -> jfieldID
       {
        assert(clazz = jni::Unwrap(classValue.Ptr()));
        assert(name == std::string("peer"));
        assert(sig == std::string("J"));
        return jni::Unwrap(objectFieldID.Ptr());
       };

    env.functions->GetLongField = [] (JNIEnv*, jobject obj, jfieldID fieldID) -> jlong
       {
        assert(obj = jni::Unwrap(objectValue.Ptr()));
        assert(fieldID = jni::Unwrap(objectFieldID.Ptr()));
        return reinterpret_cast<jlong>(&peerInstance);
       };

    env.functions->RegisterNatives = [] (JNIEnv*, jclass, const JNINativeMethod* m, jint len) -> jint
       {
        assert(len <= 6);
        std::copy(m, m + len, methods);
        return JNI_OK;
       };

    #define METHOD(name, MethodPtr) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    jni::RegisterNativePeer<Peer>(env, testClass, "peer",
        METHOD("true", &Peer::True),
        METHOD("false", &Peer::False),
        METHOD("void", &Peer::Void),
        METHOD("static", &Peer::Static),
        jni::MakeNativePeerMethod("static", [] (JNIEnv&, Peer&) {}));

    assert(methods[0].name == std::string("true"));
    assert(methods[1].name == std::string("false"));
    assert(methods[2].name == std::string("void"));
    assert(reinterpret_cast<jboolean (*)(JNIEnv&, jobject)>(methods[0].fnPtr)(env, jni::Unwrap(objectValue.Ptr())) == jni::jni_true);
    assert(reinterpret_cast<jboolean (*)(JNIEnv&, jobject)>(methods[1].fnPtr)(env, jni::Unwrap(objectValue.Ptr())) == jni::jni_false);
    reinterpret_cast<void (*)(JNIEnv&, jobject, jboolean)>(methods[2].fnPtr)(env, jni::Unwrap(objectValue.Ptr()), jni::jni_true);

    jni::RegisterNativePeer<Peer>(env, testClass, "peer",
        std::make_unique<Peer, jni::JNIEnv&, jni::jboolean>,
        "initialize",
        "finalize",
        METHOD("true", &Peer::True),
        METHOD("false", &Peer::False),
        METHOD("void", &Peer::Void),
        jni::MakeNativePeerMethod("static", [] (JNIEnv&, Peer&) {}));

    assert(methods[0].name == std::string("initialize"));
    assert(methods[1].name == std::string("finalize"));

    return 0;
   }
