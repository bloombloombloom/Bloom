#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <functional>

namespace Targets::RiscV
{
    enum class IsaBase: std::uint8_t
    {
        RV32I,
        RV32E,
        RV64I,
        RV64E,
    };

    /**
     * I've not covered all standard extensions here, as I really don't think Bloom will ever need them. If that ever
     * changes, I will add the others as and when I need them.
     */
    enum class IsaExtension: std::uint8_t
    {
        INTEGER_MULTIPLICATION_DIVISION,
        ATOMICS,
        SINGLE_PRECISION_FLOATING_POINT,
        DOUBLE_PRECISION_FLOATING_POINT,
        BIT_MANIPULATION,
        COMPRESSED_INSTRUCTIONS,
        CRYPTOGRAPHY,
        CSR_INSTRUCTIONS,
        INSTRUCTION_FETCH_FENCE,
    };

    struct IsaVersionNumber
    {
        std::uint16_t major = 0;
        std::uint16_t minor = 0;
    };

    struct IsaBaseDescriptor
    {
        IsaBase base;
        IsaVersionNumber versionNumber;
    };

    struct IsaExtensionDescriptor
    {
        IsaExtension extension;
        IsaVersionNumber versionNumber;
    };

    struct IsaDescriptor
    {
        IsaBaseDescriptor baseDescriptor;
        std::unordered_map<IsaExtension, IsaExtensionDescriptor> extensionDescriptorsByExtension;

        explicit IsaDescriptor(std::string_view isaString);

        bool hasExtension(IsaExtension extension) const;
        bool isReduced() const;

        std::optional<std::reference_wrapper<const IsaExtensionDescriptor>> tryGetExtensionDescriptor(
            IsaExtension extension
        ) const;
        const IsaExtensionDescriptor& getExtensionDescriptor(IsaExtension extension) const;

    private:
        IsaDescriptor(std::string_view isaStringLower, std::size_t stringOffset);

        static IsaBaseDescriptor extractBaseDescriptorFromIsaString(
            std::string_view isaStringLower,
            std::size_t& stringOffset
        );
        static std::unordered_map<IsaExtension, IsaExtensionDescriptor> extractExtensionDescriptorsFromIsaString(
            std::string_view isaStringLower,
            std::size_t& stringOffset
        );
        static std::optional<IsaVersionNumber> extractVersionNumberFromIsaString(
            std::string_view isaStringLower,
            std::size_t& stringOffset
        );
    };
}
