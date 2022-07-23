#include "String.hpp"

#include <algorithm>
#include <cctype>

namespace Bloom
{
    std::string String::asciiToLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char character) {
            return std::tolower(character);
        });

        return str;
    }

    std::string String::asciiToUpper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char character) {
            return std::toupper(character);
        });

        return str;
    }

    bool String::isAscii(const std::string& str) {
        return !std::any_of(str.begin(), str.end(), [] (unsigned char character) {
            return character > 127;
        });
    }
}
