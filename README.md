jni.hpp is a modern, type-safe, header-only, C++14  wrapper for [JNI (Java Native Interface)](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/). Its aim is to make calling Java from C++, or C++ from Java, convenient and safe, without sacrificing low-level control, as you commonly do with code generation approaches.

Two levels of wrappers are provided.

## Low-level wrappers

The low-level wrappers match JNI, type-for-type and function-for-function, with modern C++ equivalents in a `jni` namespace. The matching uniformly follows a set of rules.

**Rules for Names**
* Everything in jni.hpp is in the `jni` namespace.
* When a name exists in JNI, the same name is used in the namespace.
* When the original name is a macro, a lowercase name is used instead, e.g. `jni::jni_ok` for `JNI_OK`.

**Rules for Types**
* Types corresponding to Java primitive types (`jni::jboolean`, `jni::jint`, `jni::jdouble`, etc.) are matched unchanged.
* Types corresponding to Java reference types (`jni::jobject`, `jni::jclass`, `jni::jarray`, etc.) are  non-copyable and non-constructible. Their references and pointers automatically convert to base types as expected. (These types do not hide pointer-ness behind a confusing typedef, like the underlying JNI headers do.)
* `jni::jsize` is `std::size_t`, not a signed integer. jni.hpp checks for overflow in the necessary places.
* Ownership types are instantiations of `unique_ptr`, and `using` declarations are provided for each specific instantiation, e.g. `jni::UniqueGlobalRef`.
* Families of functions, such as `Call*Method` and `CallStatic*Method`, are matched with a single template function, such as `jni::CallMethod<ResultType>` and `jni::CallStaticMethod<ResultType>`.
* `jni::jvalue` and the `Call*MethodV` and `Call*MethodA` function families are made redundant by type safety, and omitted.

**Rules for Parameters and Results**
* Parameters are passed, and results returned, by value or reference, not by pointer. An exception is made for possibly-null `jni::jobject` parameters and results, which _are_ pointers.
* When transferring ownership out of a function, the return type is an ownership type.
* When transferring ownership into a function, the parameter type is an ownership type.
* Output parameters are returned, not passed as pointer-to-pointer. When there are multiple outputs, they are returned as a `std::tuple`.
* Whenever a function receives paired "pointer to `T` and length" parameters, an overload is provided that accepts a statically-sized array, `std::array<T>`, `std::vector<T>`, or (if `T` is a character type) `std::basic_string<T>` argument. These overloads compute the length automatically.
* In string-related functions, `char16_t` replaces `jchar`.

**Rules for Error Handling**

* Errors are thrown, not returned or otherwise left to be explicitly checked.
* Pending Java exceptions are checked after every JNI call that may produce a pending Java exception, and thrown as `jni::PendingJavaException`.
* [Invocation API](https://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html#wp9502) errors are thrown as `std::system_error`s containing `std::error_code`s with a custom category.

## High-level wrappers

The high-level wrappers provide additional syntactic convenience and type safety when working with non-primitive Java types. They are built around the concept of _class tags_. A class tag is a C++ type that provides a static `Name` method returning a fully-qualified Java class name, thus associating a unique C++ type with the corresponding Java class. For example:

```C++
struct Math { static constexpr auto Name() { return "java/lang/Math"; } };
```

The high-level wrappers consist of a set of classes templated on class tag:

* `jni::Class<Tag>`, a wrapper for a reference to the Java class associated with the tag.
* `jni::Object<Tag>`, a wrapper for a (possibly-null) reference to an instance of the Java class associated with the tag.
* `jni::Array<E>`, a wrapper for a reference to a Java array. The element type `E` is a jni.hpp primitive type or `Object<Tag>`.
* `jni::Constructor<Tag, Args...>`, a `Callable` type wrapping a reference to a constructor for the the Java class associated with the tag. The result type `R` and each argument type in `Args` is a jni.hpp primitive type or `Object<Tag>`.
* `jni::Method<Tag, R (Args...)>`, a `Callable` type wrapping a reference to an instance method of the Java class associated with the tag. The result type `R` and each argument type in `Args` is a jni.hpp primitive type or `Object<Tag>`.
* `jni::StaticMethod<Tag, R (Args...)>`, a `Callable` type wrapping a reference to a static method of the Java class associated with the tag. The result type `R` and each argument type in `Args` is a jni.hpp primitive type or `Object<Tag>`.
* `jni::Field<Tag, T>`, a type representing an instance field of the Java class associated with the tag, and providing `Get` and `Set` methods. The field type `T` is a jni.hpp primitive type or `Object<Tag>`.
* `jni::StaticField<Tag, T>`, a type representing a static field of the Java class associated with the tag, and providing `Get` and `Set` methods. The field type `T` is a jni.hpp primitive type or `Object<Tag>`.

Finally, the high-level wrappers provide the method `jni::NativeMethod` for binding native methods which use the high-level jni.hpp types. Given a `const char *` method name and a `Callable` object whose call signature uses high-level types, `jni::NativeMethod` returns a `jni::JNINativeMethod` struct that can be included in an array passed to `jni::RegisterNatives`. `jni::NativeMethod` takes care of:

* Deducing and constructing the JNI type signature of the native method.
* Wrapping your code in a try/catch block that handles translating native exceptions to Java exceptions.

The `Callable` object passed to `jni::NativeMethod` must be movable and uniquely typed -- lambdas are perfect.

## Example code

Example code for both the low-level and high-level wrappers is provided in [the `example` subdirectory](https://github.com/mapbox/jni.hpp/tree/master/example). This code shows the use of jni.hpp for:

* Binding native methods such that they can be called from Java.
* Calling back into Java methods from native code.

## Prior art

* Many code generation approaches. SWIG, [JavaCPP](https://github.com/bytedeco/javacpp), and so on. But jni.hpp is explicitly not a binding  generator.
* [JniHelpers](https://github.com/spotify/JniHelpers) is similar in spirit. In comparison, jni.hpp takes advantage of modern C++ features such as `unique_ptr` and variadic templates, introduces no preprocessor macros, and is header-only -- no build step required.
* I hear that Boost.Python is similar, but I did not reference it in building this library.
* The low-level wrapping strategy is inspired by [Lisa Lippincott's presentation "How to call C libraries from C++"](https://github.com/jfirebaugh/PlusPlus) from Cppcon 2014, and by her previous work ["The Nitrogen Manifesto"](http://nitric.sourceforge.net/The%20Nitrogen%20Manifesto.pdf). However jni.hpp does not use "PlusPlus", the wrapping library abstration mentioned in the presentation; I found it to be a poor economy compared to simply writing out the wrappers manually.
