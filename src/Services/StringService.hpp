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

        static std::string toHex(std::uint32_t value);
        static std::string toHex(unsigned char value);
        static std::string toHex(const std::vector<unsigned char>& data);
        static std::string toHex(const std::string& data);

        static std::vector<unsigned char> dataFromHex(const std::string& hexData);

        static std::uint64_t toUint64(const std::string& str, int base = 0);
        static std::uint32_t toUint32(const std::string& str, int base = 0);
        static std::uint16_t toUint16(const std::string& str, int base = 0);
        static std::uint8_t toUint8(const std::string& str, int base = 0);

        static std::size_t hash(const std::string& str);

        static std::vector<std::string_view> split(std::string_view str, char delimiter);
    };
}
