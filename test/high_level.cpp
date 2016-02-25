#include "test.hpp"

#include <jni/jni.hpp>

#include <cassert>
#include <iostream>

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

    struct Test { static constexpr auto Name() { return "mapbox/com/Test"; } };
    static TestEnv env;

    static Testable<jni::jclass> classValue;
    static Testable<jni::jobject> objectValue;

    static bool calledNewGlobalRef = false;

    env.functions->FindClass = [] (JNIEnv*, const char* name) -> jclass
       {
        assert(name == Test::Name());
        return Unwrap(classValue.Ptr());
       };

    env.functions->NewGlobalRef = [] (JNIEnv*, jobject obj) -> jobject
       {
        calledNewGlobalRef = true;
        return obj;
       };

    env.functions->DeleteGlobalRef = [] (JNIEnv*, jobject) -> void
       {
       };

    jni::Class<Test> testClass { env };

    assert(classValue == *testClass);
    assert(calledNewGlobalRef);


    /// Constructor

    jni::Object<Test> object { objectValue.Ptr() };

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

    jni::Constructor<Test>                    defaultNew { env, testClass };
    jni::Constructor<Test, jni::jboolean>     booleanNew { env, testClass };
    jni::Constructor<Test, jni::Object<Test>> objectNew  { env, testClass };

    assert(defaultNew(env, testClass).Get() == object.Get());
    assert(booleanNew(env, testClass, true).Get() == object.Get());
    assert(objectNew(env, testClass, object).Get() == object.Get());


    /// StaticField

    static Testable<jni::jfieldID> booleanFieldID;
    static Testable<jni::jfieldID> objectFieldID;
    static const char * booleanFieldName = "boolean";
    static const char * objectFieldName = "object";

    env.functions->GetStaticFieldID = [] (JNIEnv*, jclass, const char* name, const char* sig) -> jfieldID
       {
        if (name == booleanFieldName)
           {
            assert(sig == std::string("Z"));
            return jni::Unwrap(booleanFieldID.Ptr());
           }
        else
           {
            assert(sig == std::string("Lmapbox/com/Test;"));
            return jni::Unwrap(objectFieldID.Ptr());
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

    jni::StaticField<Test, jni::jboolean>     booleanStaticField { env, testClass, booleanFieldName };
    jni::StaticField<Test, jni::Object<Test>> objectStaticField  { env, testClass, objectFieldName };

    assert(booleanStaticField.Get(env, testClass) == true);
    assert(objectStaticField.Get(env, testClass).Get() == object.Get());
    booleanStaticField.Set(env, testClass, false);
    objectStaticField.Set(env, testClass, object);


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
        assert(field == jni::Unwrap(objectFieldID.Ptr()));
        return jni::Unwrap(objectValue.Ptr());
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
        assert(field == jni::Unwrap(objectFieldID.Ptr()));
        assert(value == jni::Unwrap(objectValue.Ptr()));
       };

    jni::Field<Test, jni::jboolean>     booleanField { env, testClass, booleanFieldName };
    jni::Field<Test, jni::Object<Test>> objectField  { env, testClass, objectFieldName };
    assert(booleanField.Get(env, object) == true);
    assert(objectField.Get(env, object).Get() == object.Get());
    booleanField.Set(env, object, false);
    objectField.Set(env, object, object);


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

    jni::StaticMethod<Test, void ()>                  voidVoidStaticMethod     { env, testClass, voidMethodName };
    jni::StaticMethod<Test, void (jni::jboolean)>     voidBooleanStaticMethod  { env, testClass, voidMethodName };
    jni::StaticMethod<Test, void (jni::Object<Test>)> voidObjectStaticMethod   { env, testClass, voidMethodName };

    voidVoidStaticMethod(env, testClass);
    voidBooleanStaticMethod(env, testClass, true);
    voidObjectStaticMethod(env, testClass, object);

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

    jni::StaticMethod<Test, jni::jboolean ()>                  booleanVoidStaticMethod     { env, testClass, booleanMethodName };
    jni::StaticMethod<Test, jni::jboolean (jni::jboolean)>     booleanBooleanStaticMethod  { env, testClass, booleanMethodName };
    jni::StaticMethod<Test, jni::jboolean (jni::Object<Test>)> booleanObjectStaticMethod   { env, testClass, booleanMethodName };

    assert(booleanVoidStaticMethod(env, testClass) == true);
    assert(booleanBooleanStaticMethod(env, testClass, true) == true);
    assert(booleanObjectStaticMethod(env, testClass, object) == true);

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

    jni::StaticMethod<Test, jni::Object<Test> ()>                  objectVoidStaticMethod     { env, testClass, objectMethodName };
    jni::StaticMethod<Test, jni::Object<Test> (jni::jboolean)>     objectBooleanStaticMethod  { env, testClass, objectMethodName };
    jni::StaticMethod<Test, jni::Object<Test> (jni::Object<Test>)> objectObjectStaticMethod   { env, testClass, objectMethodName };

    assert(objectVoidStaticMethod(env, testClass).Get() == object.Get());
    assert(objectBooleanStaticMethod(env, testClass, true).Get() == object.Get());
    assert(objectObjectStaticMethod(env, testClass, object).Get() == object.Get());


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

    jni::Method<Test, void ()>                  voidVoidMethod     { env, testClass, voidMethodName };
    jni::Method<Test, void (jni::jboolean)>     voidBooleanMethod  { env, testClass, voidMethodName };
    jni::Method<Test, void (jni::Object<Test>)> voidObjectMethod   { env, testClass, voidMethodName };

    voidVoidMethod(env, object);
    voidBooleanMethod(env, object, true);
    voidObjectMethod(env, object, object);

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

    jni::Method<Test, jni::jboolean ()>                  booleanVoidMethod     { env, testClass, booleanMethodName };
    jni::Method<Test, jni::jboolean (jni::jboolean)>     booleanBooleanMethod  { env, testClass, booleanMethodName };
    jni::Method<Test, jni::jboolean (jni::Object<Test>)> booleanObjectMethod   { env, testClass, booleanMethodName };

    assert(booleanVoidMethod(env, object) == true);
    assert(booleanBooleanMethod(env, object, true) == true);
    assert(booleanObjectMethod(env, object, object) == true);

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

    jni::Method<Test, jni::Object<Test> ()>                  objectVoidMethod     { env, testClass, objectMethodName };
    jni::Method<Test, jni::Object<Test> (jni::jboolean)>     objectBooleanMethod  { env, testClass, objectMethodName };
    jni::Method<Test, jni::Object<Test> (jni::Object<Test>)> objectObjectMethod   { env, testClass, objectMethodName };

    assert(objectVoidMethod(env, object).Get() == object.Get());
    assert(objectBooleanMethod(env, object, true).Get() == object.Get());
    assert(objectObjectMethod(env, object, object).Get() == object.Get());


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

        auto voidVoid = jni::NativeMethod("voidVoid", [] (JNIEnv&, ObjectOrClass) {});
        assert(voidVoid.name == std::string("voidVoid"));
        assert(voidVoid.signature == std::string("()V"));
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(voidVoid.fnPtr)(&env, nullptr);

        auto trueVoid = jni::NativeMethod("true", [] (JNIEnv&, ObjectOrClass) { return jni::jni_true; });
        assert(trueVoid.signature == std::string("()Z"));
        assert(reinterpret_cast<jboolean (*)(JNIEnv*, jobject)>(trueVoid.fnPtr)(&env, nullptr) == JNI_TRUE);

        auto falseVoid = jni::NativeMethod("false", [] (JNIEnv&, ObjectOrClass) { return jni::jni_false; });
        assert(falseVoid.signature == std::string("()Z"));
        assert(reinterpret_cast<jboolean (*)(JNIEnv*, jobject)>(falseVoid.fnPtr)(&env, nullptr) == JNI_FALSE);

        auto voidTrue = jni::NativeMethod("voidTrue", [] (JNIEnv&, ObjectOrClass, jni::jboolean b) { assert( b); });
        assert(voidTrue.signature == std::string("(Z)V"));
        reinterpret_cast<jboolean (*)(JNIEnv*, jobject, jboolean)>(voidTrue.fnPtr)(&env, nullptr, JNI_TRUE);

        auto voidFalse = jni::NativeMethod("voidFalse", [] (JNIEnv&, ObjectOrClass, jni::jboolean b) { assert(!b); });
        assert(voidFalse.signature == std::string("(Z)V"));
        reinterpret_cast<jboolean (*)(JNIEnv*, jobject, jboolean)>(voidFalse.fnPtr)(&env, nullptr, JNI_FALSE);

        auto voidObject = jni::NativeMethod("voidObject", [] (JNIEnv&, ObjectOrClass, jni::Object<Test>) {});
        assert(voidObject.signature == std::string("(Lmapbox/com/Test;)V"));
        reinterpret_cast<jboolean (*)(JNIEnv*, jobject, jobject)>(voidObject.fnPtr)(&env, nullptr, nullptr);

        auto objectVoid = jni::NativeMethod("objectVoid", [] (JNIEnv&, ObjectOrClass) -> jni::Object<Test> { return nullptr; });
        assert(objectVoid.signature == std::string("()Lmapbox/com/Test;"));
        assert(reinterpret_cast<jobject (*)(JNIEnv*, jobject)>(objectVoid.fnPtr)(&env, nullptr) == nullptr);

        auto objectObject = jni::NativeMethod("objectObject", [] (JNIEnv&, ObjectOrClass, jni::Object<Test>) -> jni::Object<Test> { return nullptr; });
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

        auto throwsException = jni::NativeMethod("throwsException", [] (JNIEnv&, ObjectOrClass) { throw std::runtime_error("test"); });
        lastExceptionMessage = nullptr;
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(throwsException.fnPtr)(&env, nullptr);
        assert(lastExceptionMessage == std::string("test"));

        auto throwsUnknown = jni::NativeMethod("throwsUnknown", [] (JNIEnv&, ObjectOrClass) { throw Test(); });
        lastExceptionMessage = nullptr;
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(throwsUnknown.fnPtr)(&env, nullptr);
        assert(lastExceptionMessage == std::string("unknown native exception"));

        auto javaException = jni::NativeMethod("javaException", [] (JNIEnv&, ObjectOrClass) { jni::ThrowNew(env, jni::FindClass(env, "java/lang/Error"), "Java exception"); });
        lastExceptionMessage = nullptr;
        reinterpret_cast<void (*)(JNIEnv*, jobject)>(javaException.fnPtr)(&env, nullptr);
        assert(lastExceptionMessage == std::string("Java exception"));
       };

    TestNativeMethod(object);
    TestNativeMethod(testClass);


    /// Make

    static Testable<jni::jstring> stringValue;

    env.functions->NewString = [] (JNIEnv*, const jchar*, jsize) -> jstring
       {
        return jni::Unwrap(stringValue.Ptr());
       };

    env.functions->GetStringLength = [] (JNIEnv*, jstring string) -> jsize
       {
        assert(string == jni::Unwrap(stringValue.Ptr()));
        return 5;
       };

    env.functions->GetStringRegion = [] (JNIEnv*, jstring string, jsize start, jsize len, jchar* buf)
       {
        assert(string == jni::Unwrap(stringValue.Ptr()));
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


    return 0;
   }
