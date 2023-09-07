#pragma once

#include <cstdint>

#include "src/Exceptions/Exception.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Targets::Microchip::Avr::Avr8Bit::OpcodeDecoder::Exceptions
{
class DecodeFailure: public ::Exceptions::Exception
    {
    public:
        Targets::TargetMemoryAddress byteAddress;
        std::uint32_t opcode;

        explicit DecodeFailure(Targets::TargetMemoryAddress byteAddress, std::uint32_t opcode)
            : ::Exceptions::Exception("Failed to decode AVR opcode")
            , byteAddress(byteAddress)
            , opcode(opcode)
        {}
    };
}
