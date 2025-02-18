#pragma once

#include <cstdint>

namespace Targets::RiscV::Opcodes
{
    using Opcode = std::uint32_t;
    using OpcodeCompressed = std::uint16_t;

    enum class GprNumber: std::uint8_t
    {
        X0 = 0, X1 = 1, X2 = 2, X3 = 3, X4 = 4, X5 = 5, X6 = 6, X7 = 7, X8 = 8, X9 = 9, X10 = 10, X11 = 11, X12 = 12,
        X13 = 13, X14 = 14, X15 = 15, X16 = 16, X17 = 17, X18 = 18, X19 = 19, X20 = 20, X21 = 21, X22 = 22, X23 = 23,
        X24 = 24, X25 = 25, X26 = 26, X27 = 27, X28 = 28, X29 = 29, X30 = 30, X31 = 31,
    };

    static constexpr auto Ebreak = Opcode{0x00100073};
    static constexpr auto EbreakCompressed = OpcodeCompressed{0x9002};

    static constexpr auto Fence = Opcode{0x0000000F};
    static constexpr auto FenceI = Opcode{0x0000100F};
}
