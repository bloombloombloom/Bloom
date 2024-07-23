#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <set>
#include <map>

#include "src/Helpers/BiMap.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "RegisterDescriptor.hpp"

namespace DebugServer::Gdb
{
    using GdbMemoryAddress = std::uint32_t;

    /**
     * GDB target descriptor.
     */
    class TargetDescriptor
    {
    public:
        std::map<GdbRegisterId, RegisterDescriptor> gdbRegisterDescriptorsById;
        std::map<GdbRegisterId, const Targets::TargetRegisterDescriptor*> targetRegisterDescriptorsByGdbId;

        virtual ~TargetDescriptor() = default;

        /**
         * For targets with multiple address spaces (e.g. AVR), GDB encodes address space information into memory
         * addresses, by applying a mask.
         *
         * This function should identify the encoded address space within a GDB memory address, and return the
         * relevant address space descriptor.
         *
         * @param address
         * @return
         */
        virtual const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptorFromGdbAddress(
            GdbMemoryAddress address
        ) const = 0;

        /**
         * This function should translate a GDB memory address to a target memory address. This should strip any
         * GDB-specific masks and return an address that can be used within Bloom.
         *
         * @param address
         * @return
         */
        virtual Targets::TargetMemoryAddress translateGdbAddress(GdbMemoryAddress address) const = 0;

        /**
         * This function should translate a target memory address to a GDB memory address. It should encode any
         * additional data expected by GDB.
         *
         * @param address
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         * @return
         */
        virtual GdbMemoryAddress translateTargetMemoryAddress(
            Targets::TargetMemoryAddress address,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) const = 0;
    };
}
