#pragma once

#include <string>
#include <experimental/string_view>

namespace jni
   {
    inline std::u16string MakeUTF16(std::experimental::string_view utf8)
       {
        std::u16string result;
        result.reserve(utf8.size());

        // State-machine implementation based on "Flexible and Economical UTF-8 Decoder",
        // by Bjoern Hoehrmann, http://bjoern.hoehrmann.de/utf-8/decoder/dfa/.

        // If the leading byte is:
        //
        //   00..7f, then we're done
        //
        //   c2..df, leading byte for two-byte sequence
        //      Second byte must be 80..bf
        //
        //   e1..ec, leading byte for three-byte sequence
        //   ee..ef, leading byte for three-byte sequence
        //      Second and third byte must be 80..bf
        //
        //   f1..f3, leading byte for four-byte sequence
        //      Second, third, and fourth byte must be 80..bf
        //
        //   e0, leading byte for three-byte, possibly-overlong sequence
        //      Second byte must be a0..bf
        //      Third byte must be 80..bf
        //
        //   ed, leading byte for three-byte sequence with potential invalid code points
        //      Second byte must be 80..9f
        //      Third byte must be 80..bf
        //
        //   f0, leading byte for four-byte, possibly-overlong sequence
        //      Second byte must be 90..bf
        //      Third and fourth byte must be 80..bf
        //
        //   f4, leading byte for four-byte sequence with potential invalid code points
        //      Second byte must be 80..8f
        //      Third and fourth byte must be 80..bf
        //
        //   All other leading bytes are invalid. 80..bf are invalid continuation bytes.
        //   c0 and c1 are an invalid overlong sequence. f5..ff are not used in UTF-8.

        static constexpr uint8_t types[256] =
           {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 00..0f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 10..1f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 20..2f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 30..3f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 40..4f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 50..5f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 60..6f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 70..7f

            // Continuation bytes
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 80..8f
            9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,  // 90..9f
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  // a0..af
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  // b0..bf

            // Leading bytes for two-byte sequences
            8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // c0..cf
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // d0..df

            // Leading bytes for three- and four-byte sequences
            10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, // e0..ef
            11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8  // f0..ff
           };

        // A constant for transitions to the invalid state, to help valid transitions stand out in the transition table below.
        static constexpr uint8_t _ = 1;

        static constexpr uint8_t transitions[9 * 16] =
           {
            0, _, 2, 3, 5, 8, 7, _, _, _, 4, 6, _, _, _, _,  // state 0 - starting state
            _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,  // state 1 - invalid state
            _, 0, _, _, _, _, _, 0, _, 0, _, _, _, _, _, _,  // state 2 - expecting one continuation byte (types 1, 7, or 9)
            _, 2, _, _, _, _, _, 2, _, 2, _, _, _, _, _, _,  // state 3 - expecting two continuation bytes
            _, _, _, _, _, _, _, 2, _, _, _, _, _, _, _, _,  // state 4 - expecting a0..bf (type 7),      then one continuation byte
            _, 2, _, _, _, _, _, _, _, 2, _, _, _, _, _, _,  // state 5 - expecting 80..9f (type 1 or 9), then one continuation byte
            _, _, _, _, _, _, _, 3, _, 3, _, _, _, _, _, _,  // state 6 - expecting 90..bf (type 7 or 7), then two continuation bytes
            _, 3, _, _, _, _, _, 3, _, 3, _, _, _, _, _, _,  // state 7 - expecting three continuation bytes
            _, 3, _, _, _, _, _, _, _, _, _, _, _, _, _, _   // state 8 - expecting 80..8f (type 1),      then two continuation bytes
           };

        uint8_t state = 0;
        uint8_t prev = 0;
        uint32_t pt = 0;

        for (auto it = utf8.begin(); it != utf8.end(); prev = state, ++it)
           {
            const char c = *it;
            uint8_t type = types[static_cast<uint8_t>(c)];

            if (state == 0)
               {
                pt = (0xFF >> type) & c;
               }
            else
               {
                pt = (pt << 6) | (c & 0b111111);
               }

            state = transitions[state * 16 + type];

            if (state == 0)
               {
                if (pt > 0xFFFF)
                   {
                    result += static_cast<char16_t>(0xD800 + ((pt - 0x10000) >> 10));
                    result += static_cast<char16_t>(0xDC00 + (pt & 0b1111111111));
                   }
                else
                   {
                    result += static_cast<char16_t>(pt);
                   }
               }
            else if (state == 1)
               {
                result += 0xFFFD;
                state = 0;
                if (prev != 0)
                   {
                    it--;
                   }
               }
           }

        if (state != 0 && state != 1)
           {
            result += 0xFFFD;
           }

        return result;
       }

    inline std::string MakeUTF8(std::experimental::u16string_view utf16)
       {
        std::string result;
        result.reserve(utf16.size() * 3 / 2);

        static constexpr uint8_t types[256] =
           {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 00..0f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 10..1f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 20..2f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 30..3f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 40..4f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 50..5f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 60..6f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 70..7f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 80..8f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 90..9f
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // a0..af
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // b0..bf
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // c0..cf
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,  // d0..df
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // e0..ef
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // f0..ff
           };

        // A constant for transitions to the invalid state, to help valid transitions stand out in the transition table below.
        static constexpr uint8_t _ = 1;

        static constexpr uint8_t transitions[3 * 4] =
           {
            0, 2, _, _,  // state 0 - starting state
            _, _, _, _,  // state 1 - invalid state
            _, _, 0, _   // state 2 - expecting low surrogate
           };

        uint8_t state = 0;
        uint8_t prev = 0;
        uint32_t pt = 0;

        for (auto it = utf16.begin(); it != utf16.end(); prev = state, ++it)
           {
            const char16_t c = *it;
            uint8_t type = types[static_cast<uint8_t>(c >> 8)];

            if (state == 0)
               {
                pt = (0xFFFF >> (!!type * 6)) & c;
               }
            else
               {
                pt = (pt << 10) | (c & 0b1111111111);
               }

            state = transitions[state * 4 + type];

            if (state == 0)
               {
                if (pt < 0x80)
                   {
                    result += static_cast<char>(pt);
                   }
                else if (pt < 0x800)
                   {
                    result += static_cast<char>(0b11000000 |  (pt >> 6));
                    result += static_cast<char>(0b10000000 | ((pt >> 0) & 0b111111));
                   }
                else if (pt < 0x10000)
                   {
                    result += static_cast<char>(0b11100000 |  (pt >> 12));
                    result += static_cast<char>(0b10000000 | ((pt >>  6) & 0b111111));
                    result += static_cast<char>(0b10000000 | ((pt >>  0) & 0b111111));
                   }
                else
                   {
                    result += static_cast<char>(0b11110000 |  (pt >> 18));
                    result += static_cast<char>(0b10000000 | ((pt >> 12) & 0b111111));
                    result += static_cast<char>(0b10000000 | ((pt >>  6) & 0b111111));
                    result += static_cast<char>(0b10000000 | ((pt >>  0) & 0b111111));
                   }
               }
            else if (state == 1)
               {
                result += static_cast<char>(0xEF);
                result += static_cast<char>(0xBF);
                result += static_cast<char>(0xBD);
                state = 0;
                if (prev != 0)
                   {
                    it--;
                   }
               }
           }

        if (state != 0 && state != 1)
           {
            result += static_cast<char>(0xEF);
            result += static_cast<char>(0xBF);
            result += static_cast<char>(0xBD);
           }

        return result;
       }
   }
