#pragma once

#include <jni/functions.hpp>

namespace jni
   {
    // A deleter that gets the JNIEnv via GetEnv, rather than storing the value passed to the constructor.
    // The deleting thread must have a JVM attachment.
    //
    // Useful when deletion will happen on an auxiliary thread, particularly the finalizer thread. In such
    // cases, you may use one of the following:
    //
    //   low-level: UniqueGlobalRef<jobject, EnvGettingDeleter> and NewGlobalRef<EnvGettingDeleter>
    //   high-level: Global<Object<Tag>, EnvGettingDeleter> and obj.NewGlobalRef<EnvGettingDeleter>
    //
    template < RefDeletionMethod DeleteRef >
    class EnvGettingDeleter
       {
        private:
            JavaVM* vm = nullptr;

        public:
            EnvGettingDeleter() = default;
            EnvGettingDeleter(JNIEnv& e) : vm(&GetJavaVM(e)) {}

            void operator()(jobject* p) const
               {
                if (p)
                   {
                    assert(vm);
                    (GetEnv(*vm).*DeleteRef)(Unwrap(p));
                   }
               }
       };

    // A deleter that first tries GetEnv, but falls back to AttachCurrentThread if a JVM is not already attached.
    // In the latter case, it detaches after deleting the reference.
    //
    // Useful when deletion will happen on an auxiliary thread which may or may not have a JVM attachment. In such
    // cases, you may use one of the following:
    //
    //   low-level: UniqueGlobalRef<jobject, EnvAttachingDeleter> and NewGlobalRef<EnvAttachingDeleter>
    //   high-level: Global<Object<Tag>, EnvAttachingDeleter> and obj.NewGlobalRef<EnvAttachingDeleter>
    //
    template < RefDeletionMethod DeleteRef >
    class EnvAttachingDeleter
       {
        private:
            JavaVM* vm = nullptr;

        public:
            EnvAttachingDeleter() = default;
            EnvAttachingDeleter(JNIEnv& e) : vm(&GetJavaVM(e)) {}

            void operator()(jobject* p) const
               {
                if (p)
                   {
                    assert(vm);
                    JNIEnv* env = nullptr;
                    jint err = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_1);
                    if (err == JNI_OK)
                       {
                        (env->*DeleteRef)(Unwrap(p));
                       }
                    else if (err == JNI_EDETACHED)
                       {
                        ((*AttachCurrentThread(*vm)).*DeleteRef)(Unwrap(p));
                       }
                    else
                       {
                        CheckErrorCode(err);
                       }
                   }
               }
       };
   }
