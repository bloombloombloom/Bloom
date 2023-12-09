#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class PreparePartialFlashPageWrite: public Command<std::array<unsigned char, 5>>
    {
    public:
        PreparePartialFlashPageWrite(
            Targets::TargetMemoryAddress startAddress,
            std::uint8_t bytes
        )
            : Command(0x0A)
        {
            this->payload = {
                static_cast<unsigned char>(startAddress >> 24),
                static_cast<unsigned char>(startAddress >> 16),
                static_cast<unsigned char>(startAddress >> 8),
                static_cast<unsigned char>(startAddress),
                static_cast<unsigned char>(bytes),
            };
        }
    };
}
