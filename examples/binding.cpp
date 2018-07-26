#include "binding.hpp"

namespace java { namespace lang { struct Object; } }
namespace java { namespace lang { struct String; } }

namespace jni {

template <> struct Name<java::lang::Object> {
    using Tag = ObjectTag;
};

template <> struct Name<java::lang::String> {
    using Tag = StringTag;
};

} // namespace jni


namespace java {
namespace lang {

struct Object : jni::Binding<Object> {
    using Binding::Binding;
};

struct String : jni::Binding<String> {
    using Binding::Binding;

    // Java methods

    jni::jint length(jni::JNIEnv& env) {
        static const auto method = jni_GetMethod<jni::jint()>(env, "length");
        return self.Call(env, method);
    }
};

} // namespace lang
} // namespace java



struct HighLevelBinding;

namespace jni {
template <> struct Name<HighLevelBinding>::Tag {
    static constexpr auto Name() { return "HighLevelBinding"; }
};
} // namespace jni

struct HighLevelBinding : jni::Binding<HighLevelBinding> {
    using Binding::Binding;
    
    HighLevelBinding(jni::JNIEnv& env) : Binding(jni_New<>(env)) {}
    
    // Java methods
    jni::jint quadruple(jni::JNIEnv& env, jni::jint num) {
        // Call Java method
        static const auto method = jni_GetMethod<jni::jint(jni::jint)>(env, "quadruple");
        return self.Call(env, method, num);
    }
    
    // Native methods
    void greet(jni::JNIEnv&, java::lang::String args);
    void greet(jni::JNIEnv&, jni::jint args);
    static void greet(jni::JNIEnv&, jni::jdouble args);


    static void jni_Register(jni::JNIEnv& env) {
        jni::RegisterNatives(env, jni_Class(env),
            jni_Bind<void(java::lang::String)>::Method<&HighLevelBinding::greet>("greet"),
            jni_Bind<void(jni::jint)>::Method<&HighLevelBinding::greet>("greet"),
            jni_Bind<void(jni::jdouble)>::StaticMethod<&HighLevelBinding::greet>("greet")
        );
    }
};


void HighLevelBinding::greet(jni::JNIEnv& env, java::lang::String args) {
    const auto test = jni::Make<std::string>(env, args);
    fprintf(stderr, "greet '%s' (length %d)\n", test.c_str(), args.length(env));
}

void HighLevelBinding::greet(jni::JNIEnv& env, jni::jint args) {
    fprintf(stderr, "greet %d\n", quadruple(env, args));
}

void HighLevelBinding::greet(jni::JNIEnv&, jni::jdouble args) {
    fprintf(stderr, "greet static %f\n", args);
}



extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    jni::JNIEnv& env{ jni::GetEnv(*vm) };
    try {
        java::lang::Object::jni_Register(env);
        java::lang::String::jni_Register(env);
        HighLevelBinding::jni_Register(env);
    } catch (const jni::PendingJavaException&) {
        jni::ExceptionDescribe(env);
    }
    return jni::Unwrap(jni::jni_version_1_2);
}
