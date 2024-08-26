#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <vector>

namespace Services
{
    class StringService
    {
    public:
        static std::string asciiToLower(std::string str);

        static std::string asciiToUpper(std::string str);

        static bool isAscii(const std::string& str);
        static std::string replaceUnprintable(std::string str);
        static std::string padRight(std::string str, char padChar, std::size_t padSize);

        static std::string toHex(std::uint64_t value);
        static std::string toHex(std::uint32_t value);
        static std::string toHex(unsigned char value);
        static std::string toHex(const std::vector<unsigned char>& data);
        static std::string toHex(const std::string& data);

        static std::string toBinaryStringWithMask(std::uint64_t value, std::uint64_t mask);

        static std::vector<unsigned char> dataFromHex(const std::string& hexData);

        static std::uint64_t toUint64(const std::string& str, int base = 0);
        static std::uint32_t toUint32(const std::string& str, int base = 0);
        static std::uint16_t toUint16(const std::string& str, int base = 0);
        static std::uint8_t toUint8(const std::string& str, int base = 0);

        /**
         * Generates a unique integer from a given string.
         *
         * This function will return the same value for matching strings.
         * That is: If strA == strB, then generateUniqueInteger(strA) == generateUniqueInteger(strB).
         *
         * This function does *not* return a hash of the string. We can't use hashes because they're not unique due to
         * collisions.
         *
         * The returned value will only be unique for the duration of the current execution cycle.
         * That means generateUniqueInteger(strA) != generateUniqueInteger(strA) if the functions are called within
         * different execution cycles.
         *
         * The generated value isn't actually derived from the string itself. It's just an integer that we produce and
         * keep record of, in a key-value store. See the implementation for more.
         *
         * The purpose of this function is to generate internal integer IDs for entities within Bloom, such as address
         * spaces and memory segments. These IDs have no meaning to the user or any other external entity.
         *
         * @param str
         * @return
         */
        static std::size_t generateUniqueInteger(const std::string& str);

        static std::vector<std::string_view> split(std::string_view str, char delimiter);

        enum class TerminalColor: std::uint8_t
        {
            BLACK,
            DARK_RED,
            DARK_GREEN,
            DARK_YELLOW,
            DARK_BLUE,
            DARK_MAGENTA,
            DARK_CYAN,
            LIGHT_GRAY,
            DARK_GRAY,
            RED,
            GREEN,
            ORANGE,
            BLUE,
            MAGENTA,
            CYAN,
            WHITE,
        };

        static std::string applyTerminalColor(const std::string& string, TerminalColor color);
    };
}
