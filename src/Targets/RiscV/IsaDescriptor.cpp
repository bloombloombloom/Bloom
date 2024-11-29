#include "IsaDescriptor.hpp"

#include <array>
#include <cctype>

#include "src/Services/StringService.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Targets::RiscV
{
    using Exceptions::Exception;

    IsaDescriptor::IsaDescriptor(std::string_view isaString)
        : IsaDescriptor(Services::StringService::asciiToLower(isaString), 0)
    {}

    bool IsaDescriptor::hasExtension(IsaExtension extension) const {
        return this->extensionDescriptorsByExtension.contains(extension);
    }

    bool IsaDescriptor::isReduced() const {
        return this->baseDescriptor.base == IsaBase::RV32E || this->baseDescriptor.base == IsaBase::RV64E;
    }

    std::optional<std::reference_wrapper<const IsaExtensionDescriptor>> IsaDescriptor::tryGetExtensionDescriptor(
        IsaExtension extension
    ) const {
        const auto descriptorIt = this->extensionDescriptorsByExtension.find(extension);
        if (descriptorIt == this->extensionDescriptorsByExtension.end()) {
            return std::nullopt;
        }

        return std::cref(descriptorIt->second);
    }

    const IsaExtensionDescriptor& IsaDescriptor::getExtensionDescriptor(IsaExtension extension) const {
        const auto descriptor = this->tryGetExtensionDescriptor(extension);
        if (!descriptor.has_value()) {
            throw Exception{"Failed to get extension descriptor in RISC-V ISA descriptor - extension not found"};
        }

        return descriptor->get();
    }

    IsaDescriptor::IsaDescriptor(std::string_view isaStringLower, std::size_t stringOffset)
        : baseDescriptor(IsaDescriptor::extractBaseDescriptorFromIsaString(isaStringLower, stringOffset))
        , extensionDescriptorsByExtension(
            IsaDescriptor::extractExtensionDescriptorsFromIsaString(isaStringLower, stringOffset)
        )
    {}

    IsaBaseDescriptor IsaDescriptor::extractBaseDescriptorFromIsaString(
        std::string_view isaStringLower,
        std::size_t& stringOffset
    ) {
        struct BriefBaseDescriptor
        {
            IsaBase base;
            std::string_view name;
            IsaVersionNumber defaultVersion = {2, 0};
        };

        static constexpr auto briefDescriptors = std::to_array<BriefBaseDescriptor>({
            {.base = IsaBase::RV32I, .name = "rv32i"},
            {.base = IsaBase::RV32E, .name = "rv32e"},
            {.base = IsaBase::RV64I, .name = "rv64i"},
            {.base = IsaBase::RV64E, .name = "rv64e"},
        });

        for (const auto& briefDescriptor : briefDescriptors) {
            if (isaStringLower.find(briefDescriptor.name) == 0) {
                stringOffset += briefDescriptor.name.length();

                return IsaBaseDescriptor{
                    .base = briefDescriptor.base,
                    .versionNumber = IsaDescriptor::extractVersionNumberFromIsaString(
                        isaStringLower,
                        stringOffset
                    ).value_or(briefDescriptor.defaultVersion)
                };
            }
        }

        throw Exception{"Failed to extract RISC-V ISA base from ISA string \"" + std::string{isaStringLower} + "\""};
    }

    std::unordered_map<IsaExtension, IsaExtensionDescriptor> IsaDescriptor::extractExtensionDescriptorsFromIsaString(
        std::string_view isaStringLower,
        size_t& stringOffset
    ) {
        struct BriefExtensionDescriptor
        {
            IsaExtension extension;
            std::string_view name;
            IsaVersionNumber defaultVersion = {2, 0};
        };

        static constexpr auto singleLetterBriefDescriptors = std::to_array<BriefExtensionDescriptor>({
            {.extension = IsaExtension::INTEGER_MULTIPLICATION_DIVISION, .name = "m"},
            {.extension = IsaExtension::ATOMICS, .name = "a"},
            {.extension = IsaExtension::SINGLE_PRECISION_FLOATING_POINT, .name = "f"},
            {.extension = IsaExtension::DOUBLE_PRECISION_FLOATING_POINT, .name = "d"},
            {.extension = IsaExtension::BIT_MANIPULATION, .name = "b"},
            {.extension = IsaExtension::COMPRESSED_INSTRUCTIONS, .name = "c"},
            {.extension = IsaExtension::CRYPTOGRAPHY, .name = "k"},
        });

        auto output = std::unordered_map<IsaExtension, IsaExtensionDescriptor>{};

        const auto commitMultiLetterExtension = [&output] (std::string_view extensionName) {
            static constexpr auto multiLetterBriefDescriptors = std::to_array<BriefExtensionDescriptor>({
                {.extension = IsaExtension::CSR_INSTRUCTIONS, .name = "zicsr"},
                {.extension = IsaExtension::INSTRUCTION_FETCH_FENCE, .name = "zifencei"},
            });

            for (const auto& briefDescriptor : multiLetterBriefDescriptors) {
                if (extensionName.find(briefDescriptor.name) == 0) {
                    auto offset = briefDescriptor.name.size();
                    output.emplace(
                        briefDescriptor.extension,
                        IsaExtensionDescriptor{
                            .extension = briefDescriptor.extension,
                            .versionNumber = IsaDescriptor::extractVersionNumberFromIsaString(
                                extensionName,
                                offset
                            ).value_or(briefDescriptor.defaultVersion),
                        }
                    );

                    return;
                }
            }
        };

        auto multiLetterExtension = std::optional<std::string>{};
        while (stringOffset <= (isaStringLower.length() - 1)) {
            const auto character = isaStringLower.at(stringOffset);

            if (multiLetterExtension.has_value()) {
                if (character == '_') {
                    commitMultiLetterExtension(*multiLetterExtension);
                    multiLetterExtension = std::nullopt;
                }

                multiLetterExtension->push_back(character);
                ++stringOffset;
                continue;
            }

            for (const auto& briefDescriptor : singleLetterBriefDescriptors) {
                if (character == briefDescriptor.name.at(0)) {
                    output.emplace(
                        briefDescriptor.extension,
                        IsaExtensionDescriptor{
                            .extension = briefDescriptor.extension,
                            .versionNumber = IsaDescriptor::extractVersionNumberFromIsaString(
                                isaStringLower,
                                ++stringOffset
                            ).value_or(briefDescriptor.defaultVersion),
                        }
                    );

                    goto CONTINUE_OUTER;
                }
            }

            if (character == 'g') {
                const auto versionNumber = IsaDescriptor::extractVersionNumberFromIsaString(
                    isaStringLower,
                    ++stringOffset
                ).value_or(IsaVersionNumber{2, 0});

                output.emplace(
                    IsaExtension::INTEGER_MULTIPLICATION_DIVISION,
                    IsaExtensionDescriptor{
                        .extension = IsaExtension::INTEGER_MULTIPLICATION_DIVISION,
                        .versionNumber = versionNumber
                    }
                );

                output.emplace(
                    IsaExtension::ATOMICS,
                    IsaExtensionDescriptor{.extension = IsaExtension::ATOMICS, .versionNumber = versionNumber}
                );

                output.emplace(
                    IsaExtension::DOUBLE_PRECISION_FLOATING_POINT,
                    IsaExtensionDescriptor{
                        .extension = IsaExtension::DOUBLE_PRECISION_FLOATING_POINT,
                        .versionNumber = versionNumber
                    }
                );

                output.emplace(
                    IsaExtension::INSTRUCTION_FETCH_FENCE,
                    IsaExtensionDescriptor{
                        .extension = IsaExtension::INSTRUCTION_FETCH_FENCE,
                        .versionNumber = versionNumber
                    }
                );

                continue;
            }

            if (character == 'z' || character == 'x') {
                multiLetterExtension = std::string{character};
                ++stringOffset;
                continue;
            }

            ++stringOffset;

            CONTINUE_OUTER:
            continue;
        }

        if (multiLetterExtension.has_value()) {
            commitMultiLetterExtension(*multiLetterExtension);
        }

        // Finally, handle implied extensions
        if (
            output.contains(IsaExtension::DOUBLE_PRECISION_FLOATING_POINT)
            && !output.contains(IsaExtension::SINGLE_PRECISION_FLOATING_POINT)
        ) {
            output.emplace(
                IsaExtension::SINGLE_PRECISION_FLOATING_POINT,
                IsaExtensionDescriptor{
                    .extension = IsaExtension::SINGLE_PRECISION_FLOATING_POINT,
                    .versionNumber = {2, 0}
                }
            );
        }

        if (
            output.contains(IsaExtension::SINGLE_PRECISION_FLOATING_POINT)
            && !output.contains(IsaExtension::CSR_INSTRUCTIONS)
        ) {
            output.emplace(
                IsaExtension::CSR_INSTRUCTIONS,
                IsaExtensionDescriptor{.extension = IsaExtension::CSR_INSTRUCTIONS, .versionNumber = {2, 0}}
            );
        }

        return output;
    }

    std::optional<IsaVersionNumber> IsaDescriptor::extractVersionNumberFromIsaString(
        std::string_view isaStringLower,
        size_t& stringOffset
    ) {
        static constexpr auto getNextDigitSequence = [] (std::string_view string, std::size_t offset) {
            auto sequence = std::string{};

            for (auto i = offset; i <= (string.length() - 1); ++i) {
                const auto character = string.at(i);
                if (!std::isdigit(character)) {
                    break;
                }

                sequence.push_back(character);
            }

            return !sequence.empty() ? std::optional{sequence} : std::nullopt;
        };

        if (isaStringLower.empty()) {
            return std::nullopt;
        }

        const auto majorString = getNextDigitSequence(isaStringLower, stringOffset);
        if (!majorString.has_value()) {
            return std::nullopt;
        }

        stringOffset += majorString->length();

        auto output = IsaVersionNumber{
            .major = static_cast<std::uint16_t>(std::stoul(*majorString, nullptr, 10)),
            .minor = 0
        };

        if ((stringOffset + 1) > (isaStringLower.length() - 1) || isaStringLower.at(stringOffset) != 'p') {
            // The ISA string cannot accommodate a minor number, or the 'p' delimiter is missing
            return output;
        }

        const auto minorString = getNextDigitSequence(isaStringLower, stringOffset + 1);
        if (minorString.has_value()) {
            output.minor = static_cast<std::uint16_t>(std::stoul(*minorString, nullptr, 10));
            stringOffset += minorString->length() + 1;
        }

        return output;
    }
}
