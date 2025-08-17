#include "StringService.hpp"

#include <algorithm>
#include <iterator>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ranges>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <bitset>

namespace Services
{
    std::string StringService::asciiToLower(std::string_view str) {
        auto output = std::string{};
        output.reserve(str.size());

        std::transform(str.begin(), str.end(), std::back_inserter(output), [] (unsigned char character) {
            return std::tolower(character);
        });

        return output;
    }

    std::string StringService::asciiToUpper(std::string_view str) {
        auto output = std::string{};
        output.reserve(str.size());

        std::transform(str.begin(), str.end(), std::back_inserter(output), [] (unsigned char character) {
            return std::toupper(character);
        });

        return output;
    }

    bool StringService::isAscii(std::string_view str) {
        return !std::any_of(str.begin(), str.end(), [] (unsigned char character) {
            return character > 127;
        });
    }

    std::string StringService::replaceUnprintable(std::string_view str) {
        auto output = std::string{};
        output.reserve(str.size());

        std::transform(str.begin(), str.end(), std::back_inserter(output), [] (unsigned char character) {
            return character < 32 || character > 126 ? '?' : character;
        });

        return output;
    }

    bool StringService::isNumeric(std::string_view str) {
        return !std::any_of(str.begin(), str.end(), [] (unsigned char character) {
            return !std::isdigit(character);
        });
    }

    bool StringService::isBinary(std::string_view str) {
        return !std::any_of(str.begin(), str.end(), [] (unsigned char character) {
            return character != '0' && character != '1';
        });
    }

    std::string StringService::toHex(std::span<const unsigned char> data) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0');

        for (const auto& byte : data) {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }

    std::string StringService::toHex(std::string_view data) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0');

        for (const auto& byte : data) {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }

    std::string StringService::toBinaryStringWithMask(std::uint64_t value, std::uint64_t mask) {
        auto output = std::string{};

        const auto maskBitset = std::bitset<64>{mask};
        for (auto i = std::size_t{0}; i < maskBitset.size(); ++i) {
            const auto& bit = maskBitset[i];
            if (bit) {
                output.insert(output.begin(), (value & (0x01 << i)) != 0 ? '1' : '0');
            }
        }

        return output;
    }

    std::vector<unsigned char> StringService::dataFromHex(std::string_view hexData) {
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

    std::string_view StringService::colorCode(const TerminalColor color) {
        switch (color) {
            case TerminalColor::BLACK: {
                return "\033[30m";
            }
            case TerminalColor::DARK_RED: {
                return "\033[31m";
            }
            case TerminalColor::DARK_GREEN: {
                return "\033[32m";
            }
            case TerminalColor::DARK_YELLOW: {
                return "\033[33m";
            }
            case TerminalColor::DARK_BLUE: {
                return "\033[34m";
            }
            case TerminalColor::DARK_MAGENTA: {
                return "\033[35m";
            }
            case TerminalColor::DARK_CYAN: {
                return "\033[36m";
            }
            case TerminalColor::LIGHT_GRAY: {
                return "\033[37m";
            }
            case TerminalColor::DARK_GRAY: {
                return "\033[90m";
            }
            case TerminalColor::RED: {
                return "\033[91m";
            }
            case TerminalColor::GREEN: {
                return "\033[92m";
            }
            case TerminalColor::ORANGE: {
                return "\033[93m";
            }
            case TerminalColor::BLUE: {
                return "\033[94m";
            }
            case TerminalColor::MAGENTA: {
                return "\033[95m";
            }
            case TerminalColor::CYAN: {
                return "\033[96m";
            }
            case TerminalColor::WHITE:
            default: {
                return "\033[97m";
            }
        }
    }

    std::string StringService::applyTerminalColor(const std::string& string, TerminalColor color) {
        return std::string{StringService::colorCode(color)} + string + "\033[0m";
    }

    std::string StringService::formatKey(const std::string_view key) {
        return "`" + StringService::applyTerminalColor(std::string{key}, StringService::TerminalColor::BLUE) + "`";
    }
}
