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
            return std::isdigit(character);
        });
    }

    std::string StringService::toHex(std::uint64_t value) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0') << std::setw(16) << static_cast<unsigned int>(value);
        return stream.str();
    }

    std::string StringService::toHex(std::uint32_t value) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0') << std::setw(8) << static_cast<unsigned int>(value);
        return stream.str();
    }

    std::string StringService::toHex(std::uint16_t value) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0') << std::setw(4) << static_cast<unsigned int>(value);
        return stream.str();
    }

    std::string StringService::toHex(unsigned char value) {
        auto stream = std::stringstream{};
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(value);
        return stream.str();
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

    std::string StringService::applyTerminalColor(const std::string& string, TerminalColor color) {
        auto colorCode = std::string{};

        switch (color) {
            case TerminalColor::BLACK: {
                colorCode = "\033[30m";
                break;
            }
            case TerminalColor::DARK_RED: {
                colorCode = "\033[31m";
                break;
            }
            case TerminalColor::DARK_GREEN: {
                colorCode = "\033[32m";
                break;
            }
            case TerminalColor::DARK_YELLOW: {
                colorCode = "\033[33m";
                break;
            }
            case TerminalColor::DARK_BLUE: {
                colorCode = "\033[34m";
                break;
            }
            case TerminalColor::DARK_MAGENTA: {
                colorCode = "\033[35m";
                break;
            }
            case TerminalColor::DARK_CYAN: {
                colorCode = "\033[36m";
                break;
            }
            case TerminalColor::LIGHT_GRAY: {
                colorCode = "\033[37m";
                break;
            }
            case TerminalColor::DARK_GRAY: {
                colorCode = "\033[90m";
                break;
            }
            case TerminalColor::RED: {
                colorCode = "\033[91m";
                break;
            }
            case TerminalColor::GREEN: {
                colorCode = "\033[92m";
                break;
            }
            case TerminalColor::ORANGE: {
                colorCode = "\033[93m";
                break;
            }
            case TerminalColor::BLUE: {
                colorCode = "\033[94m";
                break;
            }
            case TerminalColor::MAGENTA: {
                colorCode = "\033[95m";
                break;
            }
            case TerminalColor::CYAN: {
                colorCode = "\033[96m";
                break;
            }
            case TerminalColor::WHITE: {
                colorCode = "\033[97m";
                break;
            }
        }

        return colorCode + string + "\033[0m";
    }
}
