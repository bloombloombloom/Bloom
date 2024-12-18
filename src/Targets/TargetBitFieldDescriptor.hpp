#pragma once

#include <cstdint>
#include <string>
#include <optional>

#include "src/Helpers/Pair.hpp"

namespace Targets
{
    struct TargetBitFieldDescriptor
    {
    public:
        std::string key;
        std::string name;
        std::uint64_t mask;
        std::optional<std::string> description;

        TargetBitFieldDescriptor(
            const std::string& key,
            const std::string& name,
            std::uint64_t mask,
            std::optional<std::string> description
        );

        TargetBitFieldDescriptor& operator = (const TargetBitFieldDescriptor& other) = delete;

        TargetBitFieldDescriptor(TargetBitFieldDescriptor&& other) noexcept = default;
        TargetBitFieldDescriptor& operator = (TargetBitFieldDescriptor&& other) = default;

        std::size_t width() const;

        [[nodiscard]] TargetBitFieldDescriptor clone() const;

    private:
        TargetBitFieldDescriptor(const TargetBitFieldDescriptor& other) = default;
    };
}
