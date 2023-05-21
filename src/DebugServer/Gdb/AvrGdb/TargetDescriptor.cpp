#include "TargetDescriptor.hpp"

#include <numeric>

#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb
{
    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterType;

    using Bloom::Exceptions::Exception;

    TargetDescriptor::TargetDescriptor(const Bloom::Targets::TargetDescriptor& targetDescriptor)
        : DebugServer::Gdb::TargetDescriptor(
            targetDescriptor,
            {
                {Targets::TargetMemoryType::FLASH, 0},
                {Targets::TargetMemoryType::RAM, 0x00800000U},
                {Targets::TargetMemoryType::EEPROM, 0x00810000U},
            },
            {},
            {},
            {}
        )
    {
        this->loadRegisterMappings();
    }

    void TargetDescriptor::loadRegisterMappings() {
        const auto generalPurposeTargetRegisterDescriptorIds = this->targetDescriptor.registerDescriptorIdsForType(
            TargetRegisterType::GENERAL_PURPOSE_REGISTER
        );

        const auto statusTargetRegisterDescriptorIds = this->targetDescriptor.registerDescriptorIdsForType(
            TargetRegisterType::STATUS_REGISTER
        );

        const auto stackPointerTargetRegisterDescriptorIds = this->targetDescriptor.registerDescriptorIdsForType(
            TargetRegisterType::STACK_POINTER
        );

        if (generalPurposeTargetRegisterDescriptorIds.size() != 32) {
            throw Exception("Unexpected general purpose register count");
        }

        if (statusTargetRegisterDescriptorIds.empty()) {
            throw Exception("Missing status register descriptor");
        }

        if (stackPointerTargetRegisterDescriptorIds.empty()) {
            throw Exception("Missing stack pointer register descriptor");
        }

        /*
         * For AVR targets, GDB defines 35 registers in total:
         *
         * - Register ID 0 through 31 are general purpose registers
         * - Register ID 32 is the status register (SREG)
         * - Register ID 33 is the stack pointer register
         * - Register ID 34 is the program counter
         *
         * For AVR targets, we don't have a target register descriptor for the program counter, so we don't map that
         * GDB register ID (34) to anything here. Instead, the register command packet handlers (ReadRegisters,
         * WriteRegister, etc) will handle any operations involving that GDB register.
         */

        // General purpose registers
        GdbRegisterId gdbRegisterId = 0;
        for (const auto descriptorId : generalPurposeTargetRegisterDescriptorIds) {
            auto gdbRegisterDescriptor = RegisterDescriptor(
                gdbRegisterId,
                1,
                "General Purpose Register " + std::to_string(gdbRegisterId)
            );

            this->gdbRegisterIdsByTargetRegisterDescriptorId.emplace(descriptorId, gdbRegisterDescriptor.id);
            this->targetRegisterDescriptorIdsByGdbRegisterId.emplace(gdbRegisterDescriptor.id, descriptorId);

            this->gdbRegisterDescriptorsById.emplace(gdbRegisterDescriptor.id, std::move(gdbRegisterDescriptor));

            gdbRegisterId++;
        }

        const auto& statusTargetRegisterDescriptor = this->targetDescriptor.registerDescriptorsById.at(
            *(statusTargetRegisterDescriptorIds.begin())
        );

        auto statusGdbRegisterDescriptor = RegisterDescriptor(
            TargetDescriptor::STATUS_GDB_REGISTER_ID,
            1,
            "Status Register"
        );

        if (statusTargetRegisterDescriptor.size > statusGdbRegisterDescriptor.size) {
            throw Exception("AVR8 status target register size exceeds the GDB register size.");
        }

        this->gdbRegisterIdsByTargetRegisterDescriptorId.emplace(
            statusTargetRegisterDescriptor.id,
            statusGdbRegisterDescriptor.id
        );
        this->targetRegisterDescriptorIdsByGdbRegisterId.emplace(
            statusGdbRegisterDescriptor.id,
            statusTargetRegisterDescriptor.id
        );

        this->gdbRegisterDescriptorsById.emplace(
            statusGdbRegisterDescriptor.id,
            std::move(statusGdbRegisterDescriptor)
        );

        const auto& stackPointerTargetRegisterDescriptor = this->targetDescriptor.registerDescriptorsById.at(
            *(stackPointerTargetRegisterDescriptorIds.begin())
        );

        auto stackPointerGdbRegisterDescriptor = RegisterDescriptor(
            TargetDescriptor::STACK_POINTER_GDB_REGISTER_ID,
            2,
            "Stack Pointer Register"
        );

        if (stackPointerTargetRegisterDescriptor.size > stackPointerGdbRegisterDescriptor.size) {
            throw Exception("AVR8 stack pointer target register size exceeds the GDB register size.");
        }

        this->gdbRegisterIdsByTargetRegisterDescriptorId.emplace(
            stackPointerTargetRegisterDescriptor.id,
            stackPointerGdbRegisterDescriptor.id
        );
        this->targetRegisterDescriptorIdsByGdbRegisterId.emplace(
            stackPointerGdbRegisterDescriptor.id,
            stackPointerTargetRegisterDescriptor.id
        );

        this->gdbRegisterDescriptorsById.emplace(
            stackPointerGdbRegisterDescriptor.id,
            std::move(stackPointerGdbRegisterDescriptor)
        );

        /*
         * We acknowledge the GDB program counter register here, but we don't map it to any target register descriptors.
         *
         * This is because we can't access the program counter on AVR targets in the same way we do with other
         * registers. We don't have a register descriptor for the program counter. We have to treat it as a special
         * case in the register access command packet handlers. See CommandPackets::ReadRegister,
         * CommandPackets::WriteRegister, etc for more.
         */
        auto programCounterGdbRegisterDescriptor = RegisterDescriptor(
            TargetDescriptor::PROGRAM_COUNTER_GDB_REGISTER_ID,
            4,
            "Program Counter"
        );

        this->gdbRegisterDescriptorsById.emplace(
            programCounterGdbRegisterDescriptor.id,
            std::move(programCounterGdbRegisterDescriptor)
        );
    }
}
