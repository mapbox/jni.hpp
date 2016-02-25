CFLAGS = $(env CFLAGS) -Iinclude --std=c++14 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Werror

test:
	$(CXX) -o tst $(CFLAGS) -Itest -g -Wno-padded test/low_level.cpp && ./tst
	$(CXX) -o tst $(CFLAGS) -Itest -g -Wno-padded test/high_level.cpp && ./tst

.PHONY: test

example:
	$(CXX) -dynamiclib -o example/libhello.jnilib $(CFLAGS) -Wno-shadow -Itest example/hello.cpp
	javac example/Hello.java
	cd example && java Hello $(shell whoami)

.PHONY: example
