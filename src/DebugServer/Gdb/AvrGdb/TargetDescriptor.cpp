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
            }
        )
    {
        this->loadRegisterMappings();
    }

    std::optional<GdbRegisterNumber> TargetDescriptor::getRegisterNumberFromTargetRegisterDescriptor(
        const Targets::TargetRegisterDescriptor& registerDescriptor
    ) const {
        return this->targetRegisterDescriptorsByGdbNumber.valueAt(registerDescriptor);
    }

    const RegisterDescriptor& TargetDescriptor::getRegisterDescriptorFromNumber(GdbRegisterNumber number) const {
        if (this->registerDescriptorsByGdbNumber.contains(number)) {
            return this->registerDescriptorsByGdbNumber.at(number);
        }

        throw Exception("Unknown register from GDB - register number (" + std::to_string(number)
            + ") not mapped to any GDB register descriptor.");
    }

    const TargetRegisterDescriptor& TargetDescriptor::getTargetRegisterDescriptorFromNumber(
        GdbRegisterNumber number
    ) const {
        if (this->targetRegisterDescriptorsByGdbNumber.contains(number)) {
            return this->targetRegisterDescriptorsByGdbNumber.at(number);
        }

        throw Exception("Unknown register from GDB - register number (" + std::to_string(number)
            + ") not mapped to any target register descriptor.");
    }

    const std::vector<GdbRegisterNumber>& TargetDescriptor::getRegisterNumbers() const {
        return this->registerNumbers;
    }

    void TargetDescriptor::loadRegisterMappings() {
        auto& registerDescriptorsByType = this->targetDescriptor.registerDescriptorsByType;
        if (!registerDescriptorsByType.contains(TargetRegisterType::STATUS_REGISTER)) {
            throw Exception("Missing status register descriptor");
        }

        if (!registerDescriptorsByType.contains(TargetRegisterType::STACK_POINTER)) {
            throw Exception("Missing stack pointer register descriptor");
        }

        if (!registerDescriptorsByType.contains(TargetRegisterType::PROGRAM_COUNTER)) {
            throw Exception("Missing program counter register descriptor");
        }

        if (!registerDescriptorsByType.contains(TargetRegisterType::GENERAL_PURPOSE_REGISTER)
            || registerDescriptorsByType.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).size() != 32
        ) {
            throw Exception("Unexpected general purpose register count");
        }

        /*
         * For AVR targets, avr-gdb defines 35 registers in total:
         *
         * Register number 0 through 31 are general purpose registers
         * Register number 32 is the status register (SREG)
         * Register number 33 is the stack pointer register
         * Register number 34 is the program counter register
         */

        // Generate 35 register numbers (0 -> 34)
        std::iota(this->registerNumbers.begin(), this->registerNumbers.end(), 0);

        /*
         * Worth noting that gpRegisterDescriptors will always be sorted in the correct order, from register 0 to 31.
         *
         * Hmm, but the sorting is based on the start address (see TargetRegisterDescriptor::<() for more). So
         * effectively, we're assuming that the registers will be laid out in the correct order, in memory. I think
         * this assumption is fair.
         */
        const auto& gpRegisterDescriptors = registerDescriptorsByType.at(
            TargetRegisterType::GENERAL_PURPOSE_REGISTER
        );

        // General purpose registers
        GdbRegisterNumber regNumber = 0;
        for (const auto& descriptor : gpRegisterDescriptors) {
            this->registerDescriptorsByGdbNumber.insert(std::pair(
                regNumber,
                RegisterDescriptor(
                    regNumber,
                    1,
                    "General Purpose Register " + std::to_string(regNumber)
                )
            ));

            this->targetRegisterDescriptorsByGdbNumber.insert(std::pair(
                regNumber,
                descriptor
            ));

            regNumber++;
        }

        // Status, stack pointer and program counter registers
        const auto statusDescriptor = RegisterDescriptor(
            32,
            1,
            "Status Register"
        );

        this->registerDescriptorsByGdbNumber.insert(std::pair(statusDescriptor.number, statusDescriptor));
        this->targetRegisterDescriptorsByGdbNumber.insert(std::pair(
            statusDescriptor.number,
            *(registerDescriptorsByType.at(TargetRegisterType::STATUS_REGISTER).begin())
        ));

        const auto stackPointerDescriptor = RegisterDescriptor(
            33,
            2,
            "Stack Pointer Register"
        );

        this->registerDescriptorsByGdbNumber.insert(
            std::pair(stackPointerDescriptor.number, stackPointerDescriptor)
        );
        this->targetRegisterDescriptorsByGdbNumber.insert(std::pair(
            stackPointerDescriptor.number,
            *(registerDescriptorsByType.at(TargetRegisterType::STACK_POINTER).begin())
        ));

        const auto programCounterDescriptor = RegisterDescriptor(
            34,
            4,
            "Program Counter"
        );

        this->registerDescriptorsByGdbNumber.insert(std::pair(
            programCounterDescriptor.number,
            programCounterDescriptor
        ));
        this->targetRegisterDescriptorsByGdbNumber.insert(std::pair(
            programCounterDescriptor.number,
            *(registerDescriptorsByType.at(TargetRegisterType::PROGRAM_COUNTER).begin())
        ));

        if (registerDescriptorsByType.at(TargetRegisterType::STATUS_REGISTER).size() > statusDescriptor.size) {
            throw Exception("AVR8 status target register size exceeds the GDB register size.");
        }

        if (registerDescriptorsByType.at(TargetRegisterType::STACK_POINTER).size() > stackPointerDescriptor.size) {
            throw Exception("AVR8 stack pointer target register size exceeds the GDB register size.");
        }

        if (
            registerDescriptorsByType.at(TargetRegisterType::PROGRAM_COUNTER).size() > programCounterDescriptor.size
        ) {
            throw Exception("AVR8 program counter size exceeds the GDB register size.");
        }
    }
}
