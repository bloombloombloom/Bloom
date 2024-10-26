#pragma once

#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    class AvrGdbTargetDescriptor: public DebugServer::Gdb::TargetDescriptor
    {
    public:
        static constexpr auto SRAM_ADDRESS_MASK = 0x00800000U;
        static constexpr auto EEPROM_ADDRESS_MASK = 0x00810000U;

        static constexpr auto STATUS_GDB_REGISTER_ID = 32;
        static constexpr auto STACK_POINTER_GDB_REGISTER_ID = 33;
        static constexpr auto PROGRAM_COUNTER_GDB_REGISTER_ID = 34;

        const Targets::TargetAddressSpaceDescriptor& programAddressSpaceDescriptor;
        const Targets::TargetAddressSpaceDescriptor& eepromAddressSpaceDescriptor;
        const Targets::TargetAddressSpaceDescriptor& sramAddressSpaceDescriptor;
        const Targets::TargetAddressSpaceDescriptor& gpRegistersAddressSpaceDescriptor;

        const Targets::TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;
        const Targets::TargetMemorySegmentDescriptor& eepromMemorySegmentDescriptor;
        const Targets::TargetMemorySegmentDescriptor& sramMemorySegmentDescriptor;
        const Targets::TargetMemorySegmentDescriptor& gpRegistersMemorySegmentDescriptor;

        const Targets::TargetPeripheralDescriptor& cpuGpPeripheralDescriptor;
        const Targets::TargetRegisterGroupDescriptor& cpuGpRegisterGroupDescriptor;

        explicit AvrGdbTargetDescriptor(const Targets::TargetDescriptor& targetDescriptor);

        /**
         * For AVR targets, GDB encodes address space information into memory addresses, by applying a mask.
         *
         * This function identifies the encoded address space within the given GDB memory address, and returns the
         * relevant address space descriptor.
         *
         * @param address
         * @return
         */
        [[nodiscard]] const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptorFromGdbAddress(
            GdbMemoryAddress address
        ) const;

        /**
         * This function translates a GDB memory address to a target memory address. It will strip away any
         * GDB-specific masks and return an address that can be used within Bloom.
         *
         * @param address
         * @return
         */
        [[nodiscard]] Targets::TargetMemoryAddress translateGdbAddress(GdbMemoryAddress address) const;

        /**
         * This function translates a target memory address to a GDB memory address. It will encode address space
         * information into the address, as expected by GDB.
         *
         * @param address
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         * @return
         */
        [[nodiscard]] GdbMemoryAddress translateTargetMemoryAddress(
            Targets::TargetMemoryAddress address,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) const;
    };
}
