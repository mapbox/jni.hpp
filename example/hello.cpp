#include <jni/jni.hpp>

// The global references created during registration are purposefully leaked. Due to
// the design of Java/JNI, there is virtually no way to reliably release them. See
// http://bleaklow.com/2006/02/18/jni_onunload_mostly_useless.html for details.

static void RegisterLowLevel(JavaVM* vm)
   {
    jni::JNIEnv& env = jni::GetEnv(*vm);

    static jni::jclass* systemClass = jni::NewGlobalRef(env, jni::FindClass(env, "java/lang/System")).release();
    static jni::jclass* printStreamClass = jni::NewGlobalRef(env, jni::FindClass(env, "java/io/PrintStream")).release();

    static jni::jfieldID& outFieldID = jni::GetStaticFieldID(env, *systemClass, "out", "Ljava/io/PrintStream;");
    static jni::jmethodID& printlnMethodID = jni::GetMethodID(env, *printStreamClass, "println", "(Ljava/lang/String;)V");

    struct Greeter
       {
        static void Greet(JNIEnv* env, jobject, jobjectArray args)
           {
            try
               {
                auto greeting = std::u16string(u"Hello, ") +
                    std::get<0>(
                       jni::GetStringChars(*env,
                          reinterpret_cast<jni::jstring*>(
                              jni::GetObjectArrayElement(*env, jni::Wrap<jni::jobjectArray*>(args), 0)))).get() +
                    u" (Native Low-Level)";

                jni::CallMethod<void>(*env,
                    jni::GetStaticField<jni::jobject*>(*env, *systemClass, outFieldID),
                    printlnMethodID,
                    jni::NewString(*env, greeting));
               }
            catch (...)
               {
                jni::ThrowJavaError(*env, std::current_exception());
               }
           }
       };

    jni::RegisterNatives(env, jni::FindClass(env, "LowLevelGreeter"), {
      { "greet", "([Ljava/lang/String;)V", reinterpret_cast<void*>(&Greeter::Greet) }
    });
   };

static void RegisterHighLevel(JavaVM* vm)
   {
    struct Greeter     { static constexpr auto Name() { return "HighLevelGreeter"; } };
    struct System      { static constexpr auto Name() { return "java/lang/System"; } };
    struct PrintStream { static constexpr auto Name() { return "java/io/PrintStream"; } };

    jni::JNIEnv& env { jni::GetEnv(*vm) };

    static jni::Class<System>* systemClass = new jni::Class<System>(env);
    static jni::Class<PrintStream>* printStreamClass = new jni::Class<PrintStream>(env);

    static jni::StaticField<System, jni::Object<PrintStream>> systemOut { env, *systemClass, "out" };
    static jni::Method<PrintStream, void (jni::String)> println { env, *printStreamClass, "println" };

    jni::RegisterNatives(env, *jni::Class<Greeter>(env), {
       jni::NativeMethod("greet", [&] (jni::JNIEnv& env, jni::Object<Greeter>, jni::Array<jni::String> args)
         {
          println(env, systemOut.Get(env, *systemClass),
              jni::Make<jni::String>(env,
                  u"Hello, " + jni::Make<std::u16string>(env, args.Get(env, 0)) + u" (Native High-Level)"));
         })
    });
   };

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*)
   {
    RegisterLowLevel(vm);
    RegisterHighLevel(vm);
    return jni::Unwrap(jni::jni_version_1_2);
   }
