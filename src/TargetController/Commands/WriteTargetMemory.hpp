#pragma once

#include "Command.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentType.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class WriteTargetMemory: public Command
    {
    public:
        static constexpr CommandType type = CommandType::WRITE_TARGET_MEMORY;
        static const inline std::string name = "WriteTargetMemory";

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryBuffer buffer;

        WriteTargetMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBuffer&& buffer
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
            , startAddress(startAddress)
            , buffer(std::move(buffer))
        {};

        [[nodiscard]] CommandType getType() const override {
            return WriteTargetMemory::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::RAM;
        }
    };
}
