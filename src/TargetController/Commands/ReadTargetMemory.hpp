#pragma once

#include <cstdint>
#include <set>

#include "Command.hpp"
#include "src/TargetController/Responses/TargetMemoryRead.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentType.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetMemoryAddressRange.hpp"

namespace TargetController::Commands
{
    class ReadTargetMemory: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetMemoryRead;

        static constexpr CommandType type = CommandType::READ_TARGET_MEMORY;
        static const inline std::string name = "ReadTargetMemory";

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize bytes;

        /**
         * Currently, we only cache program memory. This flag has no effect when reading from other memories.
         */
        bool bypassCache;

        std::set<Targets::TargetMemoryAddressRange> excludedAddressRanges;

        ReadTargetMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            bool bypassCache = false,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
            , startAddress(startAddress)
            , bytes(bytes)
            , bypassCache(bypassCache)
            , excludedAddressRanges(excludedAddressRanges)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ReadTargetMemory::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::RAM;
        }
    };
}
