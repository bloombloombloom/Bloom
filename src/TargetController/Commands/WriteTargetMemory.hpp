#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/TargetMemoryRead.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::TargetController::Commands
{
    class WriteTargetMemory: public Command
    {
    public:
        static constexpr CommandType type = CommandType::WRITE_TARGET_MEMORY;
        static inline const std::string name = "WriteTargetMemory";

        Targets::TargetMemoryType memoryType;
        std::uint32_t startAddress;
        Targets::TargetMemoryBuffer buffer;

        WriteTargetMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const Targets::TargetMemoryBuffer& buffer
        )
            : memoryType(memoryType)
            , startAddress(startAddress)
            , buffer(buffer)
        {};

        [[nodiscard]] CommandType getType() const override {
            return WriteTargetMemory::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
