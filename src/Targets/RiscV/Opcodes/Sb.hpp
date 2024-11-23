#pragma once

#include <cstdint>
#include <cassert>

#include "Opcode.hpp"

namespace Targets::RiscV::Opcodes
{
    struct Sb
    {
        GprNumber baseAddressRegister;
        GprNumber valueRegister;
        std::uint16_t addressOffset;

        [[nodiscard]] constexpr Opcode opcode() const {
            assert(this->addressOffset <= 0xFFF);

            return Opcode{0x23}
                | static_cast<Opcode>(this->addressOffset & 0x1F) << 7
                | static_cast<Opcode>(this->baseAddressRegister) << 15
                | static_cast<Opcode>(this->valueRegister) << 20
                | static_cast<Opcode>((this->addressOffset >> 5) & 0x7F) << 25
            ;
        }
    };
}
