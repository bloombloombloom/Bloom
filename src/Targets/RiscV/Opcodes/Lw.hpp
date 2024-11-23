#pragma once

#include <cstdint>
#include <cassert>

#include "Opcode.hpp"

namespace Targets::RiscV::Opcodes
{
    struct Lw
    {
        GprNumber destinationRegister;
        GprNumber baseAddressRegister;
        std::uint16_t addressOffset;

        [[nodiscard]] constexpr Opcode opcode() const {
            assert(this->addressOffset <= 0xFFF);

            return Opcode{0x2003}
                | static_cast<Opcode>(this->destinationRegister) << 7
                | static_cast<Opcode>(this->baseAddressRegister) << 15
                | static_cast<Opcode>(this->addressOffset & 0xFFF) << 20
            ;
        }
    };
}
