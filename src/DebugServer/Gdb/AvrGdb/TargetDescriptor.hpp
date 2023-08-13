#pragma once

#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb
{
    class TargetDescriptor: public DebugServer::Gdb::TargetDescriptor
    {
    public:
        static constexpr auto STATUS_GDB_REGISTER_ID = 32;
        static constexpr auto STACK_POINTER_GDB_REGISTER_ID = 33;
        static constexpr auto PROGRAM_COUNTER_GDB_REGISTER_ID = 34;

        explicit TargetDescriptor(const Targets::TargetDescriptor& targetDescriptor);

    private:
        /**
         * For AVR targets, avr-gdb defines 35 registers in total:
         *
         * Register number 0 through 31 are general purpose registers
         * Register number 32 is the status register (SREG)
         * Register number 33 is the stack pointer register
         * Register number 34 is the program counter register
         *
         * This function will prepare the appropriate GDB register numbers and mappings.
         */
        void loadRegisterMappings();
    };
}
