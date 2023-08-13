#include "StringService.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

namespace Services
{
    std::string StringService::asciiToLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char character) {
            return std::tolower(character);
        });

        return str;
    }

    std::string StringService::asciiToUpper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char character) {
            return std::toupper(character);
        });

        return str;
    }

    bool StringService::isAscii(const std::string& str) {
        return !std::any_of(str.begin(), str.end(), [] (unsigned char character) {
            return character > 127;
        });
    }

    std::string StringService::replaceUnprintable(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char character) {
            return character < 32 || character > 126 ? '?' : character;
        });

        return str;
    }

    std::string StringService::toHex(unsigned char value) {
        auto stream = std::stringstream();
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(value);
        return stream.str();
    }

    std::string StringService::toHex(const std::vector<unsigned char>& data) {
        auto stream = std::stringstream();
        stream << std::hex << std::setfill('0');

        for (const auto& byte : data) {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }

    std::string StringService::toHex(const std::string& data) {
        std::stringstream stream;
        stream << std::hex << std::setfill('0');

        for (const auto& byte : data) {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }
}
