#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Commands/Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class PreparePartialFlashBlockWrite: public Command<std::array<unsigned char, 5>>
    {
    public:
        PreparePartialFlashBlockWrite(Targets::TargetMemoryAddress startAddress, std::uint8_t bytes)
            : Command(
                0x0A,
                {
                    static_cast<unsigned char>(startAddress >> 24),
                    static_cast<unsigned char>(startAddress >> 16),
                    static_cast<unsigned char>(startAddress >> 8),
                    static_cast<unsigned char>(startAddress),
                    static_cast<unsigned char>(bytes),
                }
            )
        {}
    };
}
