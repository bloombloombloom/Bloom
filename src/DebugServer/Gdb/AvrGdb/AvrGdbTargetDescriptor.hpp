#pragma once

#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetDescriptor.hpp"
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

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptorFromGdbAddress(
            GdbMemoryAddress address
        ) const override;

        Targets::TargetMemoryAddress translateGdbAddress(GdbMemoryAddress address) const override;
        GdbMemoryAddress translateTargetMemoryAddress(
            Targets::TargetMemoryAddress address,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) const override;
    };
}
