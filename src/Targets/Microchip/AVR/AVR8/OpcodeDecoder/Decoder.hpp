#pragma once

#include <cstdint>
#include <map>
#include <array>
#include <functional>
#include <optional>

#include "Instruction.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Services/BitsetService.hpp"

namespace Targets::Microchip::Avr::Avr8Bit::OpcodeDecoder
{
    class Decoder
    {
    public:
        using InstructionMapping = std::map<Targets::TargetMemoryAddress, std::optional<Instruction>>;

        /**
         * Attempts to decode AVR8 opcodes.
         *
         * @param startByteAddress
         *  The start (byte) address of the memory given via the `data` param. This is used to calculate the address
         *  of each instruction.
         *
         * @param data
         *  The opcodes to decode. This is expected to be in LSB form, which is how opcodes are stored in AVR program
         *  memory.
         *
         * @param throwOnFailure
         *  If true, this function will throw a DecodeFailure exception, upon the first decode failure.
         *
         * @return
         *  A mapping of std::optional<Instruction>, by their byte address. std::nullopt will be used for decode
         *  failures (assuming `throwOnFailure` is false)
         */
        static InstructionMapping decode(
            Targets::TargetMemoryAddress startByteAddress,
            const Targets::TargetMemoryBuffer& data,
            bool throwOnFailure = false
        );

    private:
        using OpcodeDecoderFunction = std::function<
            std::optional<Instruction>(
                const Targets::TargetMemoryBuffer::const_iterator&,
                const Targets::TargetMemoryBuffer::const_iterator&
           )
        >;
        using OpcodeDecoders = std::array<Decoder::OpcodeDecoderFunction, 144>;

        static OpcodeDecoders opcodeDecoders();
    };
}
