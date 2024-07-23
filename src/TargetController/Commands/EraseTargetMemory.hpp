#pragma once

#include "Command.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentType.hpp"

namespace TargetController::Commands
{
    class EraseTargetMemory: public Command
    {
    public:
        static constexpr CommandType type = CommandType::ERASE_TARGET_MEMORY;
        static const inline std::string name = "EraseTargetMemory";

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;

        EraseTargetMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
        {};

        [[nodiscard]] CommandType getType() const override {
            return EraseTargetMemory::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::RAM;
        }
    };
}
