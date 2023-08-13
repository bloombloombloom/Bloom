#pragma once

#include <string>
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

        static std::string toHex(unsigned char value);
        static std::string toHex(const std::vector<unsigned char>& data);
        static std::string toHex(const std::string& data);
    };
}
