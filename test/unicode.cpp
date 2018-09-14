#include <jni/unicode.hpp>

#include <cassert>

using namespace std::literals::string_literals;

int main()
   {
    assert(jni::MakeUTF16(u8"") == u"");

    for (char16_t c = 0; c <= 0x7F; ++c)
       {
        char c8 = static_cast<char>(c);
        assert(jni::MakeUTF16(std::experimental::string_view(&c8, 1)) == std::u16string(&c, 1));
       }

    for (char16_t c = 0x80; c <= 0xC2; ++c)
       {
        char c8 = static_cast<char>(c);
        assert(jni::MakeUTF16(std::experimental::string_view(&c8, 1)) == u"\xFFFD");
       }

    assert(jni::MakeUTF16(u8"abcd") == u"abcd");
    assert(jni::MakeUTF16(u8"Hello world, Καλημέρα κόσμε, コンニチハ") == u"Hello world, Καλημέρα κόσμε, コンニチハ");

    assert(jni::MakeUTF16(u8"\xED\xA0") == u"\xFFFD\xFFFD");
    assert(jni::MakeUTF16(u8"\xED\xA0\x80") == u"\xFFFD\xFFFD\xFFFD");
   }
