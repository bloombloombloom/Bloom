#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class SetFlashWriteRegion: public Command<std::array<unsigned char, 8>>
    {
    public:
        SetFlashWriteRegion(Targets::TargetMemoryAddress startAddress, Targets::TargetMemorySize bytes)
            : Command(0x01)
        {
            this->payload = {
                static_cast<unsigned char>(startAddress >> 24),
                static_cast<unsigned char>(startAddress >> 16),
                static_cast<unsigned char>(startAddress >> 8),
                static_cast<unsigned char>(startAddress),
                static_cast<unsigned char>(bytes >> 24),
                static_cast<unsigned char>(bytes >> 16),
                static_cast<unsigned char>(bytes >> 8),
                static_cast<unsigned char>(bytes),
            };
        }
    };
}
