#pragma once

#include "Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class EraseTargetMemory: public Command
    {
    public:
        static constexpr CommandType type = CommandType::ERASE_TARGET_MEMORY;
        static const inline std::string name = "EraseTargetMemory";

        Targets::TargetMemoryType memoryType;

        EraseTargetMemory(Targets::TargetMemoryType memoryType)
            : memoryType(memoryType)
        {};

        [[nodiscard]] CommandType getType() const override {
            return EraseTargetMemory::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return this->memoryType == Targets::TargetMemoryType::RAM;
        }
    };
}
