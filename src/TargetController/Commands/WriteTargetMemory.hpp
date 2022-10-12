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
        static const inline std::string name = "WriteTargetMemory";

        Targets::TargetMemoryType memoryType;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryBuffer buffer;

        WriteTargetMemory(
            Targets::TargetMemoryType memoryType,
            Targets::TargetMemoryAddress startAddress,
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

        [[nodiscard]] bool requiresDebugMode() const override {
            return this->memoryType == Targets::TargetMemoryType::RAM;
        }
    };
}
