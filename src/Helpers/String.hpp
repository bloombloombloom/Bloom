#pragma once

#include <string>

namespace Bloom
{
    class String
    {
    public:
        static std::string asciiToLower(std::string str);

        static std::string asciiToUpper(std::string str);

        static bool isAscii(const std::string& str);
    };
}
