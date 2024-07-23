#include "TargetDescriptor.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterType;

    using Exceptions::Exception;

    TargetDescriptor::TargetDescriptor(const Targets::TargetDescriptor& targetDescriptor)
        : programAddressSpaceDescriptor(targetDescriptor.getAddressSpaceDescriptor("prog"))
        , eepromAddressSpaceDescriptor(targetDescriptor.getFirstAddressSpaceDescriptorContainingMemorySegment("internal_eeprom"))
        , sramAddressSpaceDescriptor(targetDescriptor.getAddressSpaceDescriptor("data"))
        , gpRegistersAddressSpaceDescriptor(targetDescriptor.getFirstAddressSpaceDescriptorContainingMemorySegment("gp_registers"))
        , programMemorySegmentDescriptor(this->programAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_program_memory"))
        , eepromMemorySegmentDescriptor(this->eepromAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_eeprom"))
        , sramMemorySegmentDescriptor(this->sramAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_ram"))
        , gpRegistersMemorySegmentDescriptor(this->gpRegistersAddressSpaceDescriptor.getMemorySegmentDescriptor("gp_registers"))
        , cpuGpPeripheralDescriptor(targetDescriptor.getPeripheralDescriptor("cpu_gpr"))
        , cpuGpRegisterGroupDescriptor(this->cpuGpPeripheralDescriptor.getRegisterGroupDescriptor("gpr"))
    {
        /*
         * For AVR targets, GDB defines 35 registers in total:
         *
         * - Register ID 0 through 31 are general purpose registers
         * - Register ID 32 is the status register (SREG)
         * - Register ID 33 is the stack pointer register (SP)
         * - Register ID 34 is the program counter (PC)
         */

        // Create the GDB register descriptors and populate the mappings for the general purpose registers (ID 0->31)
        for (const auto& [key, descriptor] : this->cpuGpRegisterGroupDescriptor.registerDescriptorsByKey) {
            if (descriptor.type != TargetRegisterType::GENERAL_PURPOSE_REGISTER) {
                continue;
            }

            const auto gdbRegisterId = static_cast<GdbRegisterId>(
                descriptor.startAddress - this->gpRegistersMemorySegmentDescriptor.addressRange.startAddress
            );

            this->gdbRegisterDescriptorsById.emplace(gdbRegisterId, RegisterDescriptor{gdbRegisterId, 1});
            this->targetRegisterDescriptorsByGdbId.emplace(gdbRegisterId, &descriptor);
        }

        this->gdbRegisterDescriptorsById.emplace(
            TargetDescriptor::STATUS_GDB_REGISTER_ID,
            RegisterDescriptor{TargetDescriptor::STATUS_GDB_REGISTER_ID, 1}
        );
        this->targetRegisterDescriptorsByGdbId.emplace(
            TargetDescriptor::STATUS_GDB_REGISTER_ID,
            &(targetDescriptor.getPeripheralDescriptor("cpu").getRegisterGroupDescriptor("cpu")
                .getRegisterDescriptor("sreg"))
        );

        /*
         * We don't map the SP and PC GDB register IDs to target register descriptors because of inconsistencies.
         * The register command handlers will deal with these registers separately. See CommandPackets::ReadRegister,
         * CommandPackets::WriteRegister, etc for more.
         */
        this->gdbRegisterDescriptorsById.emplace(
            TargetDescriptor::STACK_POINTER_GDB_REGISTER_ID,
            RegisterDescriptor{TargetDescriptor::STACK_POINTER_GDB_REGISTER_ID, 2}
        );

        this->gdbRegisterDescriptorsById.emplace(
            TargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID,
            RegisterDescriptor{TargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID, 4}
        );
    }

    const Targets::TargetAddressSpaceDescriptor& TargetDescriptor::addressSpaceDescriptorFromGdbAddress(
        GdbMemoryAddress address
    ) const {
        if ((address & TargetDescriptor::EEPROM_ADDRESS_MASK) == TargetDescriptor::EEPROM_ADDRESS_MASK) {
            return this->eepromAddressSpaceDescriptor;
        }

        if ((address & TargetDescriptor::SRAM_ADDRESS_MASK) == TargetDescriptor::SRAM_ADDRESS_MASK) {
            return this->sramAddressSpaceDescriptor;
        }

        return this->programAddressSpaceDescriptor;
    }

    Targets::TargetMemoryAddress TargetDescriptor::translateGdbAddress(GdbMemoryAddress address) const {
        if ((address & TargetDescriptor::EEPROM_ADDRESS_MASK) == TargetDescriptor::EEPROM_ADDRESS_MASK) {
            // GDB sends EEPROM addresses in relative form - convert them to absolute form.
            return this->eepromMemorySegmentDescriptor.addressRange.startAddress
                + (address & ~(TargetDescriptor::EEPROM_ADDRESS_MASK));
        }

        if ((address & TargetDescriptor::SRAM_ADDRESS_MASK) == TargetDescriptor::SRAM_ADDRESS_MASK) {
            return address & ~(TargetDescriptor::SRAM_ADDRESS_MASK);
        }

        return address;
    }

    GdbMemoryAddress TargetDescriptor::translateTargetMemoryAddress(
        Targets::TargetMemoryAddress address,
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) const {
        if (memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::FLASH) {
            return address;
        }

        if (memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::EEPROM) {
            // GDB expects EEPROM addresses in relative form
            return (address - memorySegmentDescriptor.addressRange.startAddress)
                | TargetDescriptor::EEPROM_ADDRESS_MASK;
        }

        // We assume everything else is SRAM
        return address | TargetDescriptor::SRAM_ADDRESS_MASK;
    }
}
