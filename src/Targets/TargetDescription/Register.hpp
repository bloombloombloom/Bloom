#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <map>
#include <functional>

#include "BitField.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct Register
    {
        std::string key;
        std::string name;
        std::optional<std::string> description;
        std::uint32_t offset;
        std::uint16_t size;
        std::optional<std::uint64_t> initialValue;
        std::optional<TargetRegisterAccess> access;
        std::optional<bool> alternative;
        std::map<std::string, BitField, std::less<void>> bitFieldsByKey;

        Register(
            const std::string& key,
            const std::string& name,
            const std::optional<std::string>& description,
            std::uint32_t offset,
            std::uint16_t size,
            const std::optional<std::uint64_t>& initialValue,
            const std::optional<TargetRegisterAccess>& access,
            const std::optional<bool>& alternative,
            const std::map<std::string, BitField, std::less<void>>& bitFieldsByKey
        )
            : key(key)
            , name(name)
            , description(description)
            , offset(offset)
            , size(size)
            , initialValue(initialValue)
            , access(access)
            , alternative(alternative)
            , bitFieldsByKey(bitFieldsByKey)
        {}

        std::optional<std::reference_wrapper<const BitField>> tryGetBitField(std::string_view key) const {
            const auto bitFieldIt = this->bitFieldsByKey.find(key);

            if (bitFieldIt == this->bitFieldsByKey.end()) {
                return std::nullopt;
            }

            return std::cref(bitFieldIt->second);
        }

        const BitField& getBitField(std::string_view key) const {
            const auto bitField = this->tryGetBitField(key);
            if (!bitField.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException{
                    "Failed to get bit field \"" + std::string{key} + "\" from register in TDF - bit field not found"
                };
            }

            return bitField->get();
        }
    };
}
