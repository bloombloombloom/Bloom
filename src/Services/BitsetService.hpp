#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <type_traits>

namespace Services
{
    class BitsetService
    {
    public:
        struct BitFieldRange
        {
            std::uint8_t startPosition;
            std::uint8_t length;

            constexpr BitFieldRange(std::uint8_t startPosition, std::uint8_t length)
                : startPosition(startPosition)
                , length(length)
            {}
        };

        template <std::size_t bitFieldRangeCount>
        static constexpr std::uint8_t totalBitRangeLength(const std::array<BitFieldRange, bitFieldRangeCount>& ranges) {
            auto output = std::uint8_t{0};

            for (const auto& range : ranges) {
                output += range.length;
            }

            return output;
        }

        template <typename SubjectType>
            requires std::is_integral_v<SubjectType>
        static constexpr SubjectType extractBitField(SubjectType subject, const BitFieldRange& range) {
            const auto mask = BitsetService::setBitField(SubjectType{0}, range);
            return (subject & mask) >> (range.startPosition - range.length + 1);
        }

        template <typename SubjectType, std::size_t bitFieldRangeCount>
            requires std::is_integral_v<SubjectType>
        static constexpr SubjectType extractBitField(
            SubjectType subject,
            const std::array<BitFieldRange, bitFieldRangeCount>& ranges
        ) {
            auto output = SubjectType{0};

            for (const auto& range : ranges) {
                output = (output << range.length) | BitsetService::extractBitField(subject, range);
            }

            return output;
        }

        template <typename SubjectType>
            requires std::is_integral_v<SubjectType>
        static constexpr SubjectType setBitField(SubjectType subject, const BitFieldRange& range) {
            const auto mask = static_cast<SubjectType>(
                ~(-1LL << range.length) << (range.startPosition - range.length + 1)
            );
            return subject | mask;
        }

        template <typename SubjectType, std::size_t bitFieldRangeCount>
            requires std::is_integral_v<SubjectType>
        static constexpr SubjectType setBitField(
            SubjectType subject,
            const std::array<BitFieldRange, bitFieldRangeCount>& ranges
        ) {
            for (const auto& range : ranges) {
                subject = BitsetService::setBitField(subject, range);
            }

            return subject;
        }

        template <typename SubjectType>
            requires std::is_integral_v<SubjectType>
        static constexpr SubjectType clearBitField(SubjectType subject, const BitFieldRange& range) {
            const auto mask = static_cast<SubjectType>(
                ~(~(-1LL << range.length) << (range.startPosition - range.length + 1))
            );
            return subject & mask;
        }

        template <typename SubjectType, std::size_t bitFieldRangeCount>
            requires std::is_integral_v<SubjectType>
        static constexpr SubjectType clearBitField(
            SubjectType subject,
            const std::array<BitFieldRange, bitFieldRangeCount>& ranges
        ) {
            for (const auto& range : ranges) {
                subject = BitsetService::clearBitField(subject, range);
            }

            return subject;
        }
    };
}
