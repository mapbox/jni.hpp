#pragma once

#include <jni/types.hpp>

namespace jni {
    namespace detail {
        struct AttachCurrentThreadProxy {
            using FnNew = jint (JavaVM::*)(void **penv, void *args);
            using FnOld = jint (JavaVM::*)(JNIEnv **penv, void *args);

            static jint call(JavaVM &vm, FnOld fn, JNIEnv **penv, void *args) {
                return (vm.*fn)(penv, args);
            }

            static jint call(JavaVM &vm, FnNew fn, JNIEnv **penv, void *args) {
                return (vm.*fn)(reinterpret_cast<void **>(penv), args);
            }
        };
    }

    inline jint AttachCurrentThread(JavaVM &vm, JNIEnv **penv, void *args) {
        return detail::AttachCurrentThreadProxy::call(vm, &JavaVM::AttachCurrentThread, penv, args);
    }

    namespace detail {
        struct JNINativeMethodProxyAdaptor {
            static void convert(const char *&target, const char *source) {
                target = source;
            }

            static void convert(char *&target, const char *source) {
                target = const_cast<char *>(source);
            }
        };
    }

    template<typename NativeMethod>
    inline ::JNINativeMethod adapt(NativeMethod &nm) {
        ::JNINativeMethod result;
        detail::JNINativeMethodProxyAdaptor::convert(result.name, nm.name);
        detail::JNINativeMethodProxyAdaptor::convert(result.signature, nm.signature);
        result.fnPtr = reinterpret_cast<void *>(nm.fnPtr);
        return result;
    }
}
