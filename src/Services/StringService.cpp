#include "StringService.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ranges>
#include <functional>
#include <unordered_map>
#include <mutex>

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

    std::string StringService::toHex(std::uint32_t value) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0') << std::setw(8) << static_cast<unsigned int>(value);
        return stream.str();
    }

    std::string StringService::toHex(unsigned char value) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(value);
        return stream.str();
    }

    std::string StringService::toHex(const std::vector<unsigned char>& data) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0');

        for (const auto& byte : data) {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }

    std::string StringService::toHex(const std::string& data) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0');

        for (const auto& byte : data) {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }

    std::vector<unsigned char> StringService::dataFromHex(const std::string& hexData) {
         auto output = std::vector<unsigned char>{};

        for (auto i = 0; i < hexData.size(); i += 2) {
            const auto hexByte = std::string{(hexData.begin() + i), (hexData.begin() + i + 2)};
            output.push_back(static_cast<unsigned char>(std::stoi(hexByte, nullptr, 16)));
        }

        return output;
    }

    std::uint64_t StringService::toUint64(const std::string& str, int base) {
        return static_cast<std::uint64_t>(std::stoul(str, nullptr, base));
    }

    std::uint32_t StringService::toUint32(const std::string& str, int base) {
        return static_cast<std::uint32_t>(StringService::toUint64(str, base));
    }

    std::uint16_t StringService::toUint16(const std::string& str, int base) {
        return static_cast<std::uint16_t>(StringService::toUint64(str, base));
    }

    std::uint8_t StringService::toUint8(const std::string& str, int base) {
        return static_cast<std::uint8_t>(StringService::toUint64(str, base));
    }

    std::size_t StringService::generateUniqueInteger(const std::string& str) {
        static auto mutex = std::mutex{};

        static auto lastValue = std::size_t{0};
        static auto map = std::unordered_map<std::string, std::size_t>{};

        const auto lock = std::unique_lock{mutex};

        const auto valueIt = map.find(str);
        return valueIt != map.end() ? valueIt->second : map.emplace(str, ++lastValue).first->second;
    }

    std::vector<std::string_view> StringService::split(std::string_view str, char delimiter) {
        auto range = str |
            std::ranges::views::split(delimiter) |
            std::ranges::views::transform([] (auto&& subRange) -> std::string_view {
                return std::string_view(
                    subRange.begin(),
                    static_cast<std::size_t>(std::ranges::distance(subRange))
                );
            });

        return {std::ranges::begin(range), std::ranges::end(range)};
    }
}
