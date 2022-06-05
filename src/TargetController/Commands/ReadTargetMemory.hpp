#pragma once

#include <cstdint>
#include <set>

#include "Command.hpp"
#include "src/TargetController/Responses/TargetMemoryRead.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::TargetController::Commands
{
    class ReadTargetMemory: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetMemoryRead;

        static constexpr CommandType type = CommandType::READ_TARGET_MEMORY;
        static inline const std::string name = "ReadTargetMemory";

        Targets::TargetMemoryType memoryType;
        std::uint32_t startAddress;
        std::uint32_t bytes;
        std::set<Targets::TargetMemoryAddressRange> excludedAddressRanges;

        ReadTargetMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
        )
            : memoryType(memoryType)
            , startAddress(startAddress)
            , bytes(bytes)
            , excludedAddressRanges(excludedAddressRanges)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ReadTargetMemory::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return this->memoryType == Targets::TargetMemoryType::RAM;
        }
    };
}
