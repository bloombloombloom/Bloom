#pragma once

#include "TargetControllerService.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/Microchip/Avr8/OpcodeDecoder/Instruction.hpp"
#include "src/Targets/Microchip/Avr8/OpcodeDecoder/Decoder.hpp"

namespace Services
{
    class Avr8InstructionService
    {
    public:
        /**
         * For instructions that can change program flow, this function will attempt to figure out where, in program
         * memory, the instruction may jump to.
         *
         * This isn't possible for instructions that perform an indirect jump, where they jump to some address stored
         * in some register.
         *
         * This function should only be called on instructions that can change program flow
         * (Instruction::canChangeProgramFlow == true).
         *
         * @param instruction
         *  The subject instruction.
         *
         * @param instructionAddress
         *  The byte address of the subject instruction.
         *
         * @param instructions
         *  A reference of the InstructionMapping from which the subject instruction was taken, which should, ideally,
         *  contain the subsequent instruction.
         *
         *  We need access to the subsequent instruction in order to resolve the destination of instructions that skip
         *  the subsequent instruction. For example, the SBIC instruction will skip the subsequent instruction, if a
         *  bit in an I/O register is cleared. We can't figure out where instructions like SBIC will jump to, if we
         *  don't know the size of the subsequent instruction (that will be skipped).
         *
         *  If `instructions` does not contain the subsequent instruction, we'll just return std::nullopt for
         *  instructions that skip the subsequent instruction.
         *
         *  TODO: Why can't we just pass in the next instruction here? Instead of passing in the entire mapping?
         *
         * @return
         *  The destination byte address the subject instruction may jump to, if we were able to resolve it.
         *  Otherwise, std::nullopt.
         */
        static std::optional<Targets::TargetMemoryAddress> resolveProgramDestinationAddress(
            const Targets::Microchip::Avr8::OpcodeDecoder::Instruction& instruction,
            Targets::TargetMemoryAddress instructionAddress,
            const Targets::Microchip::Avr8::OpcodeDecoder::Decoder::InstructionMapping& instructions
        );
    };
}
