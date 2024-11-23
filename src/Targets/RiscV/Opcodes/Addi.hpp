#pragma once

#include <cstdint>
#include <cassert>

#include "Opcode.hpp"

namespace Targets::RiscV::Opcodes
{
    struct Addi
    {
        GprNumber destinationRegister;
        GprNumber sourceRegister;
        std::uint16_t value;

        [[nodiscard]] constexpr Opcode opcode() const {
            assert(this->value <= 0xFFF);

            return Opcode{0x13}
                | static_cast<Opcode>(this->destinationRegister) << 7
                | static_cast<Opcode>(this->sourceRegister) << 15
                | static_cast<Opcode>(this->value & 0xFFF) << 20
            ;
        }
    };
}
