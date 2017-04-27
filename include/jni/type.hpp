#pragma once

#include <type_traits>

namespace jni
   {
    template < char... Chars > struct Type {};

    template < class Tag >
    struct TypeFactory {
    private:
        static constexpr std::size_t Size(char const* str, std::size_t count = 0) {
            return (str[0] == '\0') ? count : Size(str + 1, count + 1);
        }

        template <std::size_t... I>
        static Type<Tag::Name()[I]...> ExpandType(std::index_sequence<I...>);

    public:
        using TagType = decltype(ExpandType(std::make_index_sequence<Size(Tag::Name())>{}));
    };

    template < class Tag >
    using TypeFromTag = typename TypeFactory<Tag>::TagType;
   }
