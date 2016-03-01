CFLAGS = $(env CFLAGS) -Iinclude --std=c++14 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Werror

test:
	$(CXX) -o tst $(CFLAGS) -Itest -g -Wno-padded test/low_level.cpp && ./tst
	$(CXX) -o tst $(CFLAGS) -Itest -g -Wno-padded test/high_level.cpp && ./tst

.PHONY: test

examples:
	$(CXX) -dynamiclib -o examples/libhello.jnilib $(CFLAGS) -Wno-shadow -Wno-padded -Itest examples/hello.cpp
	javac examples/Hello.java
	cd examples && java -Xcheck:jni Hello $(shell whoami)

	$(CXX) -dynamiclib -o examples/libpeer.jnilib $(CFLAGS) -Wno-shadow -Wno-padded -Itest examples/native_peer.cpp
	javac examples/NativePeer.java
	cd examples && java -Xcheck:jni NativePeer

.PHONY: examples
